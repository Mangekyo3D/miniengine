#include "opengl/opengldevice.h"
#include "vulkan/vulkandevicefactory.h"
#include "igpubuffer.h"
#include "../bitmap.h"
#include "../Util/colorutils.h"
#include <algorithm>
#include <cassert>
#include "icommandbuffer.h"

void IDevice::addTextureStreamRequest(TextureStreamRequest req)
{
	 m_textureRequests.push_back(req);
	 m_texStreamBufferSize += req.calculateSize();
}

void IDevice::flushPendingStreamRequests(ICommandBuffer& cmd)
{
	struct FlushRequest
	{
		ITexture* tex;
		size_t    offset;
		uint16_t  width;
		uint16_t  height;
		uint8_t mipmap;
	};

	if (m_textureRequests.size() > 0)
	{
		std::vector<FlushRequest> requests;
		requests.reserve(m_textureRequests.size());

		//create a buffer object that is large enough for all the pending requests
		auto& streamBuffer = cmd.createStreamingBuffer(m_texStreamBufferSize);
		size_t offset = 0;

		if (auto lock = IGPUBuffer::CAutoLock<uint8_t>(streamBuffer))
		{
			for (auto req : m_textureRequests)
			{
				BmpReader reader;

				if (reader.openFromFile(req.m_filename.c_str(), false))
				{
					uint16_t width = reader.getWidth();
					uint16_t height = reader.getHeight();
					uint8_t currentMipmap = 0;
					uint8_t  mipLevels = 0;
					uint16_t maxDim = std::max(width, height);
					const size_t bytesPerPixel = req.m_texture->getFormatPixelSize();

					while (maxDim > 0)
					{
						maxDim >>= 1;
						++mipLevels;
					}

					// store the first level to the buffer
					for (uint32_t i = 0; i < width; ++i)
					{
						for (uint32_t j = 0; j < height; ++j)
						{
							size_t index = (i * height + j) * bytesPerPixel;
							size_t readerindex = (i * height + j) * 3;
							for (uint8_t c = 0; c < 3; ++c)
							{
								// back convert to srgb and assign to current mipmap
								lock[index + c + offset] = reader.getData()[readerindex + 2 - c];
							}
							lock[index + offset + 4] = 255;
						}
					}

					reader.reset();

					FlushRequest flReq = {
						req.m_texture,
						offset,
						width,
						height,
						currentMipmap++
					};
					requests.push_back(flReq);

					offset += width * height * bytesPerPixel;

					if (mipLevels > 1)
					{
						std::unique_ptr <uint8_t[]> oldData;
						// use pointer for access and separate unique_ptr for ownership.
						//This will all
						uint8_t* highMipData = &lock[requests.back().offset];

						uint16_t oldHeight = height;
						uint16_t oldWidth = width;
						height >>= 1;
						width >>= 1;

						while (width > 0 || height > 0)
						{
							// we can still define a mipmap as long as one coordinate is still not zero, so clamp to one
							width = std::max(width, (uint16_t)1);
							height = std::max(height, (uint16_t)1);

							std::unique_ptr <uint8_t[]> newMipData(new uint8_t [height * width * bytesPerPixel]);

							if (width == 1 && height == 1)
							{
								for (uint8_t c = 0; c < 3; ++c)
								{
									// convert and average high mipmap in linear space
									uint8_t trC = highMipData[c];
									uint8_t tlC = highMipData[bytesPerPixel + c];
									uint8_t brC = highMipData[2 * bytesPerPixel + c];
									uint8_t blC = highMipData[3 * bytesPerPixel + c];

									float nC = (colorComponentSRGBToLinear(trC / 255.0f) +
											colorComponentSRGBToLinear(tlC / 255.0f) +
											colorComponentSRGBToLinear(brC / 255.0f) +
											colorComponentSRGBToLinear(blC / 255.0f)) * 0.25f;

									// back convert to srgb and assign to current mipmap
									lock[offset + c] = newMipData[c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
								}
								lock[offset + 4] = 255;

								flReq = {
									req.m_texture,
									offset,
									width,
									height,
									currentMipmap++
								};
								requests.push_back(flReq);

								offset += bytesPerPixel;
							}
							else if (width == 1)
							{
								for (uint32_t j = 0; j < height; ++j)
								{
									float oldCoF = 0.5f + j / (float) (height - 1) * (oldHeight - 2);
									uint32_t oldCo = static_cast<uint32_t> (oldCoF);
									float    interpFac = oldCoF - oldCo;
									size_t index = j * bytesPerPixel;

									for (uint8_t c = 0; c < 3; ++c)
									{
										// convert and average high mipmap in linear space
										uint8_t tC = highMipData[oldCo * bytesPerPixel + c];
										uint8_t bC = highMipData[(oldCo + 1) * bytesPerPixel + c];

										float nC = colorComponentSRGBToLinear(tC / 255.0f) * interpFac+
												colorComponentSRGBToLinear(bC / 255.0f) * (1.0f - interpFac);
										// back convert to srgb and assign to current mipmap
										lock[offset + index + c] = newMipData[index + c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
									}
									lock[offset + index + 4] = 255;
								}

								flReq = {
									req.m_texture,
									offset,
									width,
									height,
									currentMipmap++
								};
								requests.push_back(flReq);

								offset += height * bytesPerPixel;
							}
							else if (height == 1)
							{
								for (uint32_t i = 0; i < width; ++i)
								{
									float oldCoF = 0.5f + i / (float)(width - 1) * (oldWidth - 2);
									uint32_t oldCo = static_cast<uint32_t> (oldCoF);
									float    interpFac = oldCoF - oldCo;
									size_t index = i * bytesPerPixel;

									for (uint8_t c = 0; c < 3; ++c)
									{
										// convert and average high mipmap in linear space
										uint8_t rC = highMipData[oldCo * bytesPerPixel + c];
										uint8_t lC = highMipData[(oldCo + 1) * bytesPerPixel + c];

										float nC = colorComponentSRGBToLinear(rC / 255.0f) * interpFac+
												colorComponentSRGBToLinear(lC / 255.0f) * (1.0f - interpFac);
										// back convert to srgb and assign to current mipmap
										lock[offset + index + c] = newMipData[index + c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
									}
									lock[offset + index + 4] = 255;
								}

								flReq = {
									req.m_texture,
									offset,
									width,
									height,
									currentMipmap++
								};
								requests.push_back(flReq);

								offset += width * bytesPerPixel;
							}
							else
							{
								for (uint32_t i = 0; i < width; ++i)
								{
									for (uint32_t j = 0; j < height; ++j)
									{
										float oldCoF[2] = { 0.5f + i / (float)(width - 1) * (oldWidth - 2), 0.5f + j / (float) (height - 1) * (oldHeight - 2)};
										uint32_t oldCo[2] = { static_cast<uint32_t> (oldCoF[0]), static_cast<uint32_t> (oldCoF[1])};
										float    interpFac[2] = { oldCoF[0] - oldCo[0], oldCoF[1] - oldCo[1]};
										size_t index =(i * height + j) * bytesPerPixel;

										for (uint8_t c = 0; c < 3; ++c)
										{
											// convert and average high mipmap in linear space
											uint8_t trC = highMipData[(oldCo[0] * oldHeight + oldCo[1]) * bytesPerPixel + c];
											uint8_t tlC = highMipData[(oldCo[0] * oldHeight + oldCo[1] + 1) * bytesPerPixel + c];
											uint8_t brC = highMipData[((oldCo[0] + 1) * oldHeight + oldCo[1]) * bytesPerPixel + c];
											uint8_t blC = highMipData[((oldCo[0] + 1)* oldHeight + oldCo[1] + 1) * bytesPerPixel + c];

											float nC = colorComponentSRGBToLinear(trC / 255.0f) * interpFac[0] * interpFac[1]+
													colorComponentSRGBToLinear(tlC / 255.0f) * (1.0f - interpFac[0]) * interpFac[1] +
													colorComponentSRGBToLinear(brC / 255.0f) * interpFac[0] * (1.0f - interpFac[1]) +
													colorComponentSRGBToLinear(blC / 255.0f) * (1.0f - interpFac[0]) * (1.0f - interpFac[1]);

											// back convert to srgb and assign to current mipmap
											lock[offset + index + c] = newMipData[index + c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
										}
										lock[offset + index + 4] = 255;
									}
								}

								FlushRequest flReq = {
									req.m_texture,
									offset,
									width,
									height,
									currentMipmap++
								};
								requests.push_back(flReq);

								offset += width * height * bytesPerPixel;
							}

							oldData.reset(newMipData.release());
							highMipData = oldData.get();
							oldWidth = width;
							oldHeight = height;
							height >>= 1;
							width >>= 1;
						}
					}

					assert(currentMipmap == mipLevels);
				}
			}
		}

		for (auto flReq : requests)
		{
			cmd.copyBufferToTex(flReq.tex, flReq.offset, flReq.width, flReq.height, flReq.mipmap);
		}

		m_textureRequests.clear();
		m_texStreamBufferSize = 0;
	}
}

std::unique_ptr<IDevice> IDevice::createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice)
{
	if (bVulkanDevice)
	{
		return CVulkanDeviceFactory::createDevice(win, bDebugContext);
	}
	else
	{
		return std::make_unique<COpenGLDevice> (win, bDebugContext);
	}
}

size_t TextureStreamRequest::calculateSize() const
{
	size_t size = 0;
	uint8_t numMipMaps = m_texture->getNumMipmaps();
	uint16_t width = m_texture->getWidth();
	uint16_t height = m_texture->getHeight();
	const size_t bytesPerPixel = m_texture->getFormatPixelSize();

	for (int i = numMipMaps; i > 0; --i)
	{
		size += bytesPerPixel * static_cast<size_t> (width) * static_cast<size_t> (height);
		width >>= 1;
		height >>= 1;
		width = std::max((uint16_t)1, width);
		height = std::max((uint16_t)1, height);
	}

	return size;
}

#include "texture.h"
#include "opengldevice.h"
#include "bitmap.h"
#include "Util/colorutils.h"

#include <algorithm>
#include <cassert>

CTexture::CTexture(EFormat format, uint16_t width, uint16_t height, bool bMipmapped)
	: m_width(width)
	, m_height(height)
	, m_format(format)
{
	auto& device = COpenGLDevice::get();

	if (bMipmapped)
	{
		uint16_t minDim = std::min(m_width, m_height);

		m_mipLevels = 0;

		while (minDim > 0)
		{
			minDim >>= 1;
			++m_mipLevels;
		}
	}
	else
	{
		m_mipLevels = 1;
	}

	device.glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
	device.glTextureStorage2D(m_id, m_mipLevels, formatToGLFormat(), m_width, m_height);
}

CTexture::CTexture(std::string filename, bool bMipmapped)
{
	Bitmap imageFile;

	if (imageFile.openFromFile(filename.c_str()))
	{
		auto& device = COpenGLDevice::get();
		uint8_t currentMipmap = 0;

		m_width = imageFile.getWidth();
		m_height = imageFile.getHeight();
		m_format = eSRGB8;

		if (bMipmapped)
		{
			uint16_t minDim = std::min(m_width, m_height);

			m_mipLevels = 0;

			while (minDim > 0)
			{
				minDim >>= 1;
				++m_mipLevels;
			}
		}
		else
		{
			m_mipLevels = 1;
		}

		device.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		device.glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
		device.glTextureStorage2D(m_id, m_mipLevels, formatToGLFormat(), m_width, m_height);

		device.glTextureSubImage2D(m_id, currentMipmap++, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, imageFile.getData());

		if (m_mipLevels > 1)
		{
			std::unique_ptr <uint8_t[]> highMipData(imageFile.acquireData());

			uint16_t oldHeight = m_height;
			uint16_t oldWidth = m_width;
			uint16_t height = m_height >> 1;
			uint16_t width = m_width >> 1;

			while (width > 0 || height > 0)
			{
				// we can still define a mipmap as long as one coordinate is still not zero, so clamp to one
				width = std::max(width, (uint16_t)1);
				height = std::max(height, (uint16_t)1);

				std::unique_ptr <uint8_t[]> newMipData(new uint8_t [height * width * 3]);

				if (width == 1 && height == 1)
				{
					for (uint8_t c = 0; c < 3; ++c)
					{
						// convert and average high mipmap in linear space
						uint8_t trC = highMipData[c];
						uint8_t tlC = highMipData[3 + c];
						uint8_t brC = highMipData[6 + c];
						uint8_t blC = highMipData[9 + c];

						float nC = (colorComponentSRGBToLinear(trC / 255.0f) +
								colorComponentSRGBToLinear(tlC / 255.0f) +
								colorComponentSRGBToLinear(brC / 255.0f) +
								colorComponentSRGBToLinear(blC / 255.0f)) * 0.25f;

						// back convert to srgb and assign to current mipmap
						newMipData[c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
					}
				}
				else if (width == 1)
				{
					for (uint32_t j = 0; j < height; ++j)
					{
						float oldCoF = 0.5f + j / (float) (height - 1) * (oldHeight - 2);
						uint32_t oldCo = static_cast<uint32_t> (oldCoF);
						float    interpFac = oldCoF - oldCo;
						size_t index = j * 3;

						for (uint8_t c = 0; c < 3; ++c)
						{
							// convert and average high mipmap in linear space
							uint8_t tC = highMipData[oldCo * 3 + c];
							uint8_t bC = highMipData[(oldCo + 1) * 3 + c];

							float nC = colorComponentSRGBToLinear(tC / 255.0f) * interpFac+
									colorComponentSRGBToLinear(bC / 255.0f) * (1.0f - interpFac);
							// back convert to srgb and assign to current mipmap
							newMipData[index + c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
						}
					}
				}
				else if (height == 1)
				{
					for (uint32_t i = 0; i < width; ++i)
					{
						float oldCoF = 0.5f + i / (float)(width - 1) * (oldWidth - 2);
						uint32_t oldCo = static_cast<uint32_t> (oldCoF);
						float    interpFac = oldCoF - oldCo;
						size_t index = i * 3;

						for (uint8_t c = 0; c < 3; ++c)
						{
							// convert and average high mipmap in linear space
							uint8_t rC = highMipData[oldCo * 3 + c];
							uint8_t lC = highMipData[(oldCo + 1) * 3 + c];

							float nC = colorComponentSRGBToLinear(rC / 255.0f) * interpFac+
									colorComponentSRGBToLinear(lC / 255.0f) * (1.0f - interpFac);
							// back convert to srgb and assign to current mipmap
							newMipData[index + c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
						}
					}
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
							size_t index =(i * height + j) * 3;

							for (uint8_t c = 0; c < 3; ++c)
							{
								// convert and average high mipmap in linear space
								uint8_t trC = highMipData[(oldCo[0] * oldHeight + oldCo[1]) * 3 + c];
								uint8_t tlC = highMipData[(oldCo[0] * oldHeight + oldCo[1] + 1) * 3 + c];
								uint8_t brC = highMipData[((oldCo[0] + 1) * oldHeight + oldCo[1]) * 3 + c];
								uint8_t blC = highMipData[((oldCo[0] + 1)* oldHeight + oldCo[1] + 1) * 3 + c];

								float nC = colorComponentSRGBToLinear(trC / 255.0f) * interpFac[0] * interpFac[1]+
										colorComponentSRGBToLinear(tlC / 255.0f) * (1.0f - interpFac[0]) * interpFac[1] +
										colorComponentSRGBToLinear(brC / 255.0f) * interpFac[0] * (1.0f - interpFac[1]) +
										colorComponentSRGBToLinear(blC / 255.0f) * (1.0f - interpFac[0]) * (1.0f - interpFac[1]);

								// back convert to srgb and assign to current mipmap
								newMipData[index + c] = static_cast<uint8_t> (colorComponentLinearToSRGB(nC) * 255);
							}
						}
					}
				}
				device.glTextureSubImage2D(m_id, currentMipmap++, 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, newMipData.get());

				highMipData.reset(newMipData.release());
				oldWidth = width;
				oldHeight = height;
				height >>= 1;
				width >>= 1;
			}
		}

		assert(currentMipmap == m_mipLevels);
	}
}

CTexture::~CTexture()
{
	auto& device = COpenGLDevice::get();

	device.glDeleteTextures(1, &m_id);
}

void CTexture::bind(uint8_t unit)
{
	auto& device = COpenGLDevice::get();

	device.glBindTextureUnit(unit, m_id);
}

uint32_t CTexture::formatToGLFormat()
{
	switch (m_format)
	{
		case eRGB8:
			return GL_RGB8;
		case eRGB16f:
			return GL_RGB16F;
		case eDepth32f:
			return GL_DEPTH_COMPONENT32F;
		case eSRGB8:
			return GL_SRGB8;

		default:
			return GL_RGB8;
	}
}

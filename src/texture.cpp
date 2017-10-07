#include "texture.h"
#include "cdevice.h"
#include <algorithm>
#include "bitmap.h"

CTexture::CTexture(uint16_t width, uint16_t height, bool bMipmapped)
	: m_width(width)
	, m_height(height)
{
	auto& device = IDevice::get <CDevice>();

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
	device.glTextureStorage2D(m_id, m_mipLevels, GL_RGB8, m_width, m_height);
}

CTexture::CTexture(std::string filename, bool bMipmapped)
{
	Bitmap file;

	if (file.openFromFile(filename.c_str()))
	{
		auto& device = IDevice::get <CDevice>();

		m_width = file.getWidth();
		m_height = file.getHeight();

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
		device.glTextureStorage2D(m_id, m_mipLevels, GL_RGB8, m_width, m_height);

		device.glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, file.getData());
		// TODO: implement application downsampling for mipmaps, use driver provided one for now
		device.glGenerateTextureMipmap(m_id);
	}
}

CTexture::~CTexture()
{
	auto& device = IDevice::get <CDevice>();

	device.glDeleteTextures(1, &m_id);
}

void CTexture::bind(uint8_t unit)
{
	auto& device = IDevice::get <CDevice>();

	device.glBindTextureUnit(unit, m_id);
}

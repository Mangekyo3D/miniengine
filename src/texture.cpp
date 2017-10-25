#include "texture.h"
#include "cdevice.h"
#include <algorithm>
#include "bitmap.h"

CTexture::CTexture(EFormat format, uint16_t width, uint16_t height, bool bMipmapped)
	: m_width(width)
	, m_height(height)
	, m_format(format)
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
	device.glTextureStorage2D(m_id, m_mipLevels, formatToGLFormat(), m_width, m_height);
}

CTexture::CTexture(std::string filename, bool bMipmapped)
{
	Bitmap imageFile;

	if (imageFile.openFromFile(filename.c_str()))
	{
		auto& device = IDevice::get <CDevice>();

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

		device.glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
		device.glTextureStorage2D(m_id, m_mipLevels, formatToGLFormat(), m_width, m_height);

		device.glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, imageFile.getData());
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

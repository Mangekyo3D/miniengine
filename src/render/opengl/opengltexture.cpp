#include "opengltexture.h"
#include "opengldevice.h"
#include "../../bitmap.h"
#include "../../Util/colorutils.h"

#include <algorithm>
#include <cassert>

COpenGLTexture::COpenGLTexture(EFormat format, uint16_t width, uint16_t height, bool bMipmapped)
	: ITexture(format, width, height, bMipmapped)
{
	auto& device = COpenGLDevice::get();
	device.glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
	device.glTextureStorage2D(m_id, m_mipLevels, formatToGLFormat(), m_width, m_height);
}

COpenGLTexture::~COpenGLTexture()
{
	auto& device = COpenGLDevice::get();

	device.glDeleteTextures(1, &m_id);
}

void COpenGLTexture::bind(uint8_t unit)
{
	auto& device = COpenGLDevice::get();

	device.glBindTextureUnit(unit, m_id);
}

uint32_t COpenGLTexture::formatToGLFormat()
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

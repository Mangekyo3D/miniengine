#include "opengltexture.h"
#include "opengldevice.h"
#include "../../bitmap.h"
#include "../../Util/colorutils.h"

#include <algorithm>
#include <cassert>

COpenGLTexture::COpenGLTexture(EFormat format, uint32_t usage, uint16_t width, uint16_t height, bool bMipmapped)
	: ITexture(format, usage, width, height, bMipmapped)
{
	auto& device = COpenGLDevice::get();
	device.glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
	device.glTextureStorage2D(m_id, m_mipLevels, formatToGLFormat(), static_cast<GLint> (m_width), static_cast<GLint> (m_height));
}

COpenGLTexture::~COpenGLTexture()
{
	auto& device = COpenGLDevice::get();

	device.glDeleteTextures(1, &m_id);
}

void COpenGLTexture::bind(uint32_t unit)
{
	auto& device = COpenGLDevice::get();

	device.glBindTextureUnit(unit, m_id);
}

size_t COpenGLTexture::getFormatPixelSize()
{
	switch (m_format)
	{
		case EFormat::eRGB8:
			return 4;
		case EFormat::eSRGB8:
			return 4;
		case EFormat::eRGB16f:
			return 16;
		case EFormat::eDepth32f:
			return 4;
	}
	return 4;
}

uint32_t COpenGLTexture::formatToGLFormat()
{
	switch (m_format)
	{
		case eRGB8:
			return GL_RGBA8;
		case eRGB16f:
			return GL_RGBA16F;
		case eDepth32f:
			return GL_DEPTH_COMPONENT32F;
		case eSRGB8:
			return GL_SRGB8_ALPHA8;
	}
	return GL_RGBA8;
}

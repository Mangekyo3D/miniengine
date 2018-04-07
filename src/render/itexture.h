#pragma once
#include <stdint.h>
#include <string>

class ITexture
{
	public:
		// format abstraction for the texture
		enum EFormat
		{
			eRGB8,
			eRGB16f,
			eDepth32f,
			eSRGB8,
		};

		enum EUsage
		{
			eAttachement = (1),
			eSampled     = (1 << 1)
		};

		ITexture(EFormat format, uint32_t usage, uint32_t width, uint32_t height, bool bMipmapped);
		virtual ~ITexture() {}
		uint8_t getNumMipmaps() const { return m_mipLevels; }
		EFormat getFormat() const { return m_format; }
		uint16_t getWidth() const { return m_width; }
		uint16_t getHeight() const { return m_height; }

	protected:
		uint16_t m_width;
		uint16_t m_height;
		uint8_t  m_mipLevels;
		EFormat  m_format;
		uint32_t m_usage;
};

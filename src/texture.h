#pragma once
#include <stdint.h>
#include <string>

class CTexture
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

		CTexture(EFormat format, uint16_t width, uint16_t height, bool bMipmapped = false);
		CTexture(std::string filename, bool bMipmapped = false);
		~CTexture();
		void bind(uint8_t unit);
		EFormat getFormat() const { return m_format; }
		uint32_t getID() const { return m_id; }
		uint16_t getWidth() const { return m_width; }
		uint16_t getHeight() const { return m_height; }

	private:
		uint32_t formatToGLFormat();

		uint32_t m_id;
		uint16_t m_width;
		uint16_t m_height;
		uint8_t  m_mipLevels;
		EFormat  m_format;
};

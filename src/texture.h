#pragma once
#include <stdint.h>
#include <string>

class CTexture
{
	public:
		CTexture(uint16_t width, uint16_t height, bool bMipmapped = false);
		CTexture(std::string filename, bool bMipmapped = false);
		~CTexture();
		void bind(uint8_t unit);

	private:
		uint32_t m_id;
		uint16_t m_width;
		uint16_t m_height;
		uint8_t  m_mipLevels;
};

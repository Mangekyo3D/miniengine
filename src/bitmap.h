#pragma once
#include <stdint.h>
#include <memory>

class Bitmap
{
	public:
		Bitmap() 
			: m_data(0)
			, m_length(0)
			, m_width(0)
			, m_height(0)
		{}
		bool openFromFile(const char *);
		const uint8_t* getData() {return m_data.get();}
		uint32_t getLength() { return m_length; }
		uint32_t getWidth() { return m_width; }
		uint32_t getHeight() { return m_height; }

	private:
		std::unique_ptr<uint8_t []> m_data;
		uint32_t m_length;
		uint32_t m_width;
		uint32_t m_height;
};

#pragma once
#include <stdint.h>
#include <memory>

class BmpReader
{
	public:
		BmpReader()
			: m_data(0)
			, m_width(0)
			, m_height(0)
		{}
		bool openFromFile(const char *, bool bOnlySize);
		const uint8_t* getData() {return m_data.get();}
		uint32_t getWidth() { return m_width; }
		uint32_t getHeight() { return m_height; }

		void reset()
		{
			m_width = 0;
			m_height = 0;
			m_data.reset();
		}

	private:
		std::unique_ptr<uint8_t []> m_data;
		uint32_t m_width;
		uint32_t m_height;
};

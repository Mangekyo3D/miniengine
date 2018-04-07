#pragma once
#include <stdint.h>

class ISwapchain
{
public:
	ISwapchain()
		: m_width(0)
		, m_height(0)
	{
	}
	virtual ~ISwapchain() {}

	virtual void onResize(uint32_t newWidth, uint32_t newHeight)
	{
		m_width = newWidth;
		m_height = newHeight;
	}

	void getSize(uint32_t& width, uint32_t& height)
	{
		width = m_width;
		height = m_height;
	}

	virtual void swapBuffers() = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
};


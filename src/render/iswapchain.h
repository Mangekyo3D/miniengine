#pragma once

class ISwapchain
{
public:
	virtual ~ISwapchain() {}

	virtual void swapBuffers() = 0;
};


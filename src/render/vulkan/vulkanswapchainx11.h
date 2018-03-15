#pragma once
#include "../iswapchain.h"
#include "vulkan/vulkan.h"

class GameWindow;

class CVulkanSwapchainX11 : public ISwapchain
{
public:
	CVulkanSwapchainX11(GameWindow&);
	~CVulkanSwapchainX11();
	void swapBuffers() override;

private:
	VkSurfaceKHR m_windowSurface;
};

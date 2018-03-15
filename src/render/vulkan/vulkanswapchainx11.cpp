#include "vulkanswapchainx11.h"
#include "../../OS/X11Window.h"
#include "vulkandevice.h"

#include <iostream>

CVulkanSwapchainX11::CVulkanSwapchainX11(GameWindow& w)
{
	CVulkanDevice& device = CVulkanDevice::get();

	X11Window& win = static_cast<X11Window&> (w);
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
		VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		win.getDisplay(),
		win.getWindow()
	};

	if (device.vkCreateXlibSurfaceKHR(device, &surfaceCreateInfo, nullptr, &m_windowSurface) != VK_SUCCESS)
	{
		std::cout << "Surface creation failed!" << std::endl;
	}

	if (!device.ensureDevice(m_windowSurface))
	{
		device.vkDestroySurfaceKHR(device, m_windowSurface, nullptr);
		m_windowSurface = VK_NULL_HANDLE;
		return;
	}
}

CVulkanSwapchainX11::~CVulkanSwapchainX11()
{
	CVulkanDevice& device = CVulkanDevice::get();
	device.vkDestroySurfaceKHR(device, m_windowSurface, nullptr);
}

void CVulkanSwapchainX11::swapBuffers()
{

}

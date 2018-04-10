#include "vulkanswapchain.h"
#ifdef VK_USE_PLATFORM_WIN32_KHR
#include "../../OS/Win32Window.h"
#elif defined VK_USE_PLATFORM_XLIB_KHR
#include "../../OS/X11Window.h"
#endif
#include "vulkandevice.h"
#include "vulkanbuffer.h"

#include <iostream>
#include <limits.h>

CVulkanSwapchain::CVulkanSwapchain(GameWindow& w)
	: m_swapchain(VK_NULL_HANDLE)
	, m_surface(VK_NULL_HANDLE)
{
	auto& device = CVulkanDevice::get();

#ifdef VK_USE_PLATFORM_WIN32_KHR
	Win32Window& win = static_cast<Win32Window&> (w);
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		GetModuleHandle(nullptr),
		win.getHandle()
	};

	if (device.vkCreateWin32SurfaceKHR(device, &surfaceCreateInfo, nullptr, &m_surface) != VK_SUCCESS)
	{
		std::cout << "Surface creation failed!" << std::endl;
		return;
	}

#elif defined VK_USE_PLATFORM_XLIB_KHR
	X11Window& win = static_cast<X11Window&> (w);
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
		VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		win.getDisplay(),
		win.getWindow()
	};

	if (device.vkCreateXlibSurfaceKHR(device, &surfaceCreateInfo, nullptr, &m_surface) != VK_SUCCESS)
	{
		std::cout << "Surface creation failed!" << std::endl;
		return;
	}
#endif

	if (!device.ensureDevice(m_surface))
	{
		device.vkDestroySurfaceKHR(device, m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
		return;
	}

	recreate();
}

CVulkanSwapchain::~CVulkanSwapchain()
{
	cleanup();
	auto& device = CVulkanDevice::get();

	if (m_surface)
	{
		device.vkDestroySurfaceKHR(device, m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}
}

void CVulkanSwapchain::cleanup()
{
	auto& device = CVulkanDevice::get();

	m_frames.clear();

	// clearing the frames above should mean that work has been completed anyway, but better play it safe before cleanup
	device.vkDeviceWaitIdle(device);

	for (VkImageView& imageView : m_swapchainImageviews)
	{
		if (imageView)
		{
			device.vkDestroyImageView(device, imageView, nullptr);
		}
	}
	m_swapchainImageviews.clear();
	m_swapchainImages.clear();

	for (VkSemaphore& semaphore : m_frameAcquireSemaphores)
	{
		if (semaphore)
		{
			device.vkDestroySemaphore(device, semaphore, nullptr);
		}
	}

	m_frameAcquireSemaphores.clear();

	if (m_swapchain)
	{
		device.vkDestroySwapchainKHR(device, m_swapchain, nullptr);
		m_swapchain = VK_NULL_HANDLE;
	}
}

void CVulkanSwapchain::swapBuffers()
{
	auto& device = CVulkanDevice::get();
	SFrame& frame = *m_frames[m_currentFrameIndex].get();

	// after commands have been processed, present image to presentation engine
	VkPresentInfoKHR presentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1,
		&frame.m_renderingFinishedSemaphore,
		1,
		&m_swapchain,
		&m_currentSwapchainImage,
		nullptr
	};

	VkResult result = device.vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

	switch (result)
	{
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			recreate();
			break;
		case VK_SUBOPTIMAL_KHR:
			break;

		default:
			std::cout << "Error during queue presentation" << std::endl;
			break;
	}
}

SFrame& CVulkanSwapchain::getNextFrame()
{
	auto& device = CVulkanDevice::get();
	VkSemaphore imageAvailableSemaphore = m_nextAvailableSemaphore;

	if (!imageAvailableSemaphore)
	{
		imageAvailableSemaphore  = m_frameAcquireSemaphores[--m_availableSemaphores];
	}

	VkResult result = device.vkAcquireNextImageKHR(device, m_swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &m_currentSwapchainImage);

	switch (result)
	{
		case VK_SUCCESS:
		case VK_SUBOPTIMAL_KHR:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			recreate();
			break;

		default:
			std::cout << "Error during swapchain image acquire" << std::endl;
			break;
	}

	m_currentFrameIndex = (m_currentFrameIndex + 1) % getNumberOfVirtualFrames();

	SFrame& frame = *m_frames[m_currentFrameIndex].get();

	// if we have assigned a semaphore to the frame, recycle it for the next frame
	if (frame.m_swapchainImageAvailableSemaphore)
	{
		m_nextAvailableSemaphore = frame.m_swapchainImageAvailableSemaphore;
	}
	frame.m_swapchainImageAvailableSemaphore = imageAvailableSemaphore;

	// wait for command buffer for this frame to finish and reset its fence
	device.vkWaitForFences(device, 1, &frame.m_fence, VK_TRUE, UINT_MAX);
	device.vkResetFences(device, 1, &frame.m_fence);

	frame.cleanupOrphanedData();

	frame.m_swapchainImage = m_swapchainImages[m_currentSwapchainImage];
	frame.m_imageView = m_swapchainImageviews[m_currentSwapchainImage];

	return frame;
}

void CVulkanSwapchain::recreate()
{
	auto& device = CVulkanDevice::get();

	VkSwapchainCreateInfoKHR swapchainCreateInfo;

	if (!device.getSwapchainCreationParameters(m_surface, m_swapchain, swapchainCreateInfo))
	{
		return;
	}

	m_extent = swapchainCreateInfo.imageExtent;

	VkSwapchainKHR newSwapchain;
	if (device.vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &newSwapchain) != VK_SUCCESS)
	{
		std::cout << "Swapchain creation failed" << std::endl;
	}

	// we need to keep the old swapchain around while it is still used by swapchainCreateInfo
	cleanup();

	m_swapchain = newSwapchain;

	if (device.vkGetSwapchainImagesKHR(device, m_swapchain, &m_numSwapchainImages, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to get new swapchain images" << std::endl;
	}

	// triple buffering
	m_currentFrameIndex = 0;
	m_frames.reserve(getNumberOfVirtualFrames());
	// we need a number of semaphores equal to number of frames + 1 so that we still have a semaphore to wait the next frame
	m_availableSemaphores = getNumberOfVirtualFrames() + 1;
	m_frameAcquireSemaphores.reserve(m_availableSemaphores);
	m_nextAvailableSemaphore = VK_NULL_HANDLE;
	std::vector <VkImage> newSwapChainImages(m_numSwapchainImages);

	if (device.vkGetSwapchainImagesKHR(device, m_swapchain, &m_numSwapchainImages, &newSwapChainImages[0]) != VK_SUCCESS)
	{
		std::cout << "Failed to get new swapchain images" << std::endl;
	}

	for (uint32_t i = 0; i < getNumberOfVirtualFrames(); ++i)
	{
		m_frames.push_back(std::make_unique<SFrame>());
	}

	// create semaphores
	m_frameAcquireSemaphores.assign(m_availableSemaphores, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < m_availableSemaphores; ++i)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			nullptr,
			0
		};

		if (device.vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_frameAcquireSemaphores[i]) != VK_SUCCESS)
		{
			std::cout << "Could not create semaphore" << std::endl;
		}
	}

	m_swapchainImages = newSwapChainImages;
	// we have the images, now regenerate image views and framebuffers

	m_swapchainImageviews.reserve(m_numSwapchainImages);
	m_swapchainImageviews.assign(m_numSwapchainImages, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < m_numSwapchainImages; ++i)
	{
		VkImageSubresourceRange subresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};

		VkComponentMapping componentMapping = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};

		VkImageViewCreateInfo imageviewcreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			m_swapchainImages[i],
			VK_IMAGE_VIEW_TYPE_2D,
			VK_FORMAT_B8G8R8A8_UNORM,
			componentMapping, subresourceRange
		};

		if (device.vkCreateImageView(device, &imageviewcreateInfo, nullptr, &m_swapchainImageviews[i]) != VK_SUCCESS)
		{
			std::cout << "Could not create swapchain image view" << std::endl;
		}
	}
}

SFrame::SFrame()
	: m_swapchainImage(VK_NULL_HANDLE)
	, m_imageView(VK_NULL_HANDLE)
	, m_framebuffer(VK_NULL_HANDLE)
	, m_fence(VK_NULL_HANDLE)
	, m_swapchainImageAvailableSemaphore(VK_NULL_HANDLE)
	, m_renderingFinishedSemaphore(VK_NULL_HANDLE)

{
	auto& device = CVulkanDevice::get();

	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	// create semaphores
	if (device.vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_renderingFinishedSemaphore) != VK_SUCCESS)
	{
		std::cout << "Could not create semaphore" << std::endl;
	}

	VkFenceCreateInfo fenceCreateInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr,
		VK_FENCE_CREATE_SIGNALED_BIT
	};

	if (device.vkCreateFence(device, &fenceCreateInfo, nullptr, &m_fence) != VK_SUCCESS)
	{
		std::cout << "Could not create fence" << std::endl;
	}

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		device.getGraphicsCommandPool(0),
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};

	if (device.vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &m_commandBuffer) != VK_SUCCESS)
	{
		std::cout << "Error during command buffer allocation" << std::endl;
	}
}

SFrame::~SFrame()
{
	auto& device = CVulkanDevice::get();

	if (m_fence)
	{
		device.vkWaitForFences(device, 1, &m_fence, VK_TRUE, UINT_MAX);
		device.vkDestroyFence(device, m_fence, nullptr);
		m_fence = VK_NULL_HANDLE;
	}

	if (m_framebuffer != VK_NULL_HANDLE)
	{
		device.vkDestroyFramebuffer(device, m_framebuffer, nullptr);
		m_framebuffer = VK_NULL_HANDLE;
	}


	// destroy old command buffers if they exist
	if (m_commandBuffer)
	{
		device.vkFreeCommandBuffers(device, device.getGraphicsCommandPool(0), 1, &m_commandBuffer);
		m_commandBuffer= VK_NULL_HANDLE;
	}

	device.vkDestroySemaphore(device, m_renderingFinishedSemaphore, nullptr);

	cleanupOrphanedData();
}

void SFrame::cleanupOrphanedData()
{
	m_orphanedBuffers.clear();
}

void SFrame::orphanBuffer(std::unique_ptr<CVulkanBuffer> buffer)
{
	m_orphanedBuffers.push_back(std::move(buffer));
}

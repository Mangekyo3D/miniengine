#include "vulkandevice.h"
#include "../gpubuffer.h"
#include "../../OS/GameWindow.h"
#include <iostream>
#include <cstring>

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#define LoadProcAddress GetProcAddress
#elif defined VK_USE_PLATFORM_XLIB_KHR
#include <dlfcn.h>
#include "vulkanswapchainx11.h"
#define LoadProcAddress dlsym
#endif

static VkBool32 vulkanDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData)
{
	std::cout << pMessage << std::endl;

	return VK_TRUE;
}

CVulkanDevice& CVulkanDevice::get()
{
	return static_cast<CVulkanDevice&> (*s_device);
}

VkDevice CVulkanDevice::getDevice()
{
	return m_device;
}

VkPhysicalDevice CVulkanDevice::getPhysicalDevice()
{
	return m_physicalDevice;
}

bool CVulkanDevice::ensureDevice(VkSurfaceKHR surface)
{
	uint32_t physicalDeviceCount = 0;

	if (vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
	{
		std::cout << "Devices could not be enumerated " << std::endl;
	}

	if (physicalDeviceCount == 0)
	{
		std::cout << "No Vulkan capable devices detected!" << std::endl;
		return false;
	}

	return true;
}

CVulkanDevice::CVulkanDevice(GameWindow& win, bool bDebugContext)
	: m_device(VK_NULL_HANDLE)
	, m_instance(VK_NULL_HANDLE)
	, m_debugHandle(VK_NULL_HANDLE)
{
	m_bDebugInstance = bDebugContext;
	s_device = this;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	m_librarymodule = LoadLibrary("vulkan-1.dll");
#elif defined VK_USE_PLATFORM_XLIB_KHR
	m_librarymodule = dlopen( "libvulkan.so", RTLD_NOW);
#endif
	bool bInstanceFunctionsLoaded = true;

	if (m_librarymodule == nullptr)
	{
		std::cout << "Vulkan Runtime not found" << std::endl;
	}

	#define VK_EXPORTED_FUNCTION( fun ) \
		PFN_##fun fun; \
		if (!(fun = (PFN_##fun)LoadProcAddress(m_librarymodule, #fun))) { \
		std::cout << "Could not load exported function: " << #fun << "!" << std::endl; \
		bInstanceFunctionsLoaded = false; \
	}

	#include "VulkanFunctions.inl"

	if (!bInstanceFunctionsLoaded)
	{
		return;
	}

	#undef VK_EXPORTED_FUNCTION

	#define VK_GLOBAL_FUNCTION( fun ) \
		PFN_##fun fun; \
		if (!(fun = (PFN_##fun)vkGetInstanceProcAddr(nullptr, #fun))) { \
		std::cout << "Could not load global function: " << #fun << "!" << std::endl; \
		bInstanceFunctionsLoaded = false; \
	}

	#include "VulkanFunctions.inl"

	#undef VK_GLOBAL_FUNCTION

	if (!bInstanceFunctionsLoaded)
	{
		return;
	}


	// check if extensions are supported by the implementation
	uint32_t numExtensions;
	if (vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to initilize Vulkan Extensions" << std::endl;
		bInstanceFunctionsLoaded = false; \
	}

	std::vector<VkExtensionProperties> availableExtensions(numExtensions);
	std::vector<const char*> requiredExtensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined VK_USE_PLATFORM_XLIB_KHR
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
	};

	if (m_bDebugInstance)
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	if (vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, availableExtensions.data()) != VK_SUCCESS)
	{
		std::cout << "Failed to initilize Vulkan Extensions" << std::endl;
		bInstanceFunctionsLoaded = false;
	}

	for (const char* extName : requiredExtensions)
	{
		bool bFound = false;
		for (VkExtensionProperties& extProperty : availableExtensions)
		{
			if (strcmp(extName, extProperty.extensionName) == 0)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			std::cout << "Required Vulkan Extensions not found!" << std::endl;
			bInstanceFunctionsLoaded = false;
		}
	}

	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"Vulkan Animation Test",
		VK_MAKE_VERSION(1, 0, 0),
		"TestEngine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0
	};

	std::vector <const char*> enabledLayers;
	if (m_bDebugInstance)
	{
		enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	VkInstanceCreateInfo instanceInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&appInfo,
		static_cast<uint32_t> (enabledLayers.size()),
		enabledLayers.data(),
		static_cast<uint32_t> (requiredExtensions.size()),
		requiredExtensions.data()
	};

	// Create vulkan instance
	if (vkCreateInstance(&instanceInfo, 0, &m_instance) != VK_SUCCESS)
	{
		std::cout << "Vulkan Instance creation failed" << std::endl;
		return;
	}


	#define VK_INSTANCE_FUNCTION( fun ) \
	if (!(fun = (PFN_##fun)vkGetInstanceProcAddr(m_instance, #fun))) { \
		std::cout << "Could not load instance function: " << #fun << "!" << std::endl; \
		bInstanceFunctionsLoaded = false; \
	}

	#define VK_INSTANCE_DEBUG_FUNCTION( fun ) \
	if (m_bDebugInstance && !(fun = (PFN_##fun)vkGetInstanceProcAddr(m_instance, #fun))) { \
		std::cout << "Could not load instance debug function: " << #fun << "!" << std::endl; \
		bInstanceFunctionsLoaded = false; \
	}

	#include "VulkanFunctions.inl"

	if (!bInstanceFunctionsLoaded)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = VK_NULL_HANDLE;
		return;
	}

	if (m_bDebugInstance)
	{	// enable debugging
		VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {
			VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			nullptr,
			VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
			vulkanDebugCallback,
			nullptr
		};

		vkCreateDebugReportCallbackEXT(m_instance, &debugCreateInfo, nullptr, &m_debugHandle);
	}

	// create swapchain and device now
	VkSurfaceKHR windowSurface = VK_NULL_HANDLE;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	auto swapchain = std::make_unique <CVulkanSwapchainWin32> (win);
#elif defined VK_USE_PLATFORM_XLIB_KHR
	auto swapchain = std::make_unique <CVulkanSwapchainX11> (win);
#endif
	win.assignSwapchain(std::move(swapchain));
}

CVulkanDevice::~CVulkanDevice()
{
	if (m_bDebugInstance)
	{
		vkDestroyDebugReportCallbackEXT(m_instance, m_debugHandle, nullptr);
	}

	vkDestroyInstance(m_instance, nullptr);

#ifdef VK_USE_PLATFORM_WIN32_KHR
	CloseHandle(m_librarymodule);
#elif defined VK_USE_PLATFORM_XLIB_KHR
	dlclose(m_librarymodule);
#endif
}

std::unique_ptr<IGPUBuffer> CVulkanDevice::createGPUBuffer(size_t size)
{
	return nullptr;
}


SMemoryChunk::SMemoryChunk(VkDeviceMemory memory, const size_t size)
	: m_memory(memory)
	, m_size(size)
{
	SMemoryBlock block = {
		0,
		size,
		false
	};

	m_blocks.push_back(block);
}

SMemoryChunk::~SMemoryChunk()
{
	auto& device = CVulkanDevice::get();
	VkDevice vkDevice = device.getDevice();

	for (SMemoryBlock& block : m_blocks)
	{
		if (block.m_bUsed)
		{
			std::cout << "Deleting Memory Block at offset " << block.m_offset << " which is still used!" << std::endl;
		}
	}

	if (m_memory)
	{
		device.vkFreeMemory(vkDevice, m_memory, nullptr);
	}
}

bool SMemoryChunk::allocateBlock(size_t size, size_t alignment, size_t& offset)
{
	for (size_t index = 0;  index < m_blocks.size(); ++index)
	{
		SMemoryBlock* block = &m_blocks[index];

		// search for unused block
		if (!block->m_bUsed)
		{
			// first, calculate effective size, taking alignment into account. If
			size_t residue = (alignment - (block->m_offset % alignment)) % alignment;
			size_t effectiveSize = size + residue;

			if (block->m_size >= effectiveSize)
			{
				SMemoryBlock* prevBlock = (index > 0) ? block - 1 : nullptr;

				// add the residue to the size of the previous block, if any
				if (residue > 0 && prevBlock)
				{
					prevBlock->m_size += residue;
					block->m_offset += residue;
					block->m_size -= residue;
				}

				if (size < block->m_size)
				{
					SMemoryBlock newBlock = {
						block->m_offset + size,
						block->m_size - size,
						false
					};

					auto iter = m_blocks.begin() + index + 1;
					m_blocks.insert(iter, newBlock);

					// reset block, since it may have been changed after the insertion
					block = &m_blocks[index];
				}

				block->m_size = size;
				block->m_bUsed = true;

				// return the offset of this block back
				offset = block->m_offset;

				return true;
			}
		}
	}

	return false;
}

void SMemoryChunk::freeBlock(size_t offset)
{
	for (size_t index = 0;  index < m_blocks.size(); ++index)
	{
		SMemoryBlock* block = &m_blocks[index];

		if (block->m_offset == offset)
		{
			block->m_bUsed = false;

			// attempt to coalesce adjacent blocks
			auto mergeStartIter = m_blocks.begin() + index;
			auto mergeEndIter = mergeStartIter + 1;
			bool bMerged = false;

			if (index > 0 && !m_blocks[index - 1].m_bUsed)
			{
				--mergeStartIter;
				bMerged = true;
				mergeStartIter->m_size += block->m_size;
			}

			if (index < m_blocks.size() - 1 && !m_blocks[index + 1].m_bUsed)
			{
				++mergeEndIter;
				bMerged = true;
				mergeStartIter->m_size += block->m_size;
			}

			if (bMerged)
			{
				m_blocks.erase(mergeStartIter + 1, mergeEndIter);
			}

			return;
		}
	}
}

void SMemoryChunk::delayedFreeBlock(size_t offset)
{

}

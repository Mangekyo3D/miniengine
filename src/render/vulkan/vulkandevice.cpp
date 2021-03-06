#include "vulkandevice.h"
#include "vulkanbuffer.h"
#include "vulkantexture.h"
#include "vulkanpipeline.h"
#include "vulkanrenderpass.h"
#include "vulkancommandbuffer.h"
#include "../../OS/GameWindow.h"
#include <iostream>
#include <cstring>
#include "vulkanswapchain.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#define LoadProcAddress GetProcAddress
#define CloseDll FreeLibrary
#elif defined VK_USE_PLATFORM_XLIB_KHR
#include <dlfcn.h>
#define LoadProcAddress dlsym
#define CloseDll dlclose
#endif

static VkBool32 VKAPI_PTR vulkanDebugCallback(
	VkDebugReportFlagsEXT,
	VkDebugReportObjectTypeEXT,
	uint64_t,
	size_t,
	int32_t,
	const char*,
	const char* pMessage,
	void*)
{
#ifdef WIN32
	OutputDebugStringA(pMessage);
	OutputDebugStringA("\n");
#else
	std::cerr << pMessage << std::endl;
#endif

	return VK_TRUE;
}


CVulkanDevice* CVulkanDevice::s_device = nullptr;

CVulkanDevice& CVulkanDevice::get()
{
	return *s_device;
}

std::unique_ptr<ICommandBuffer> CVulkanDevice::beginFrame(ISwapchain& currentSwapchain)
{
	return std::make_unique <CVulkanCommandBuffer> (currentSwapchain);
}

VkPhysicalDevice CVulkanDevice::getPhysicalDevice()
{
	return m_physicalDevice;
}

bool CVulkanDevice::ensureDevice(VkSurfaceKHR surface)
{
	if (m_device != VK_NULL_HANDLE)
	{
		return true;
	}

	unsigned int numDevices = 0;

	if (vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr) != VK_SUCCESS || numDevices == 0)
	{
		std::cout << "No supported Vulkan Devices found!" << std::endl;
		return false;
	}

	// we have more than one device available, let's get their properties now
	std::vector <VkPhysicalDevice> physDevices(numDevices);

	if (vkEnumeratePhysicalDevices(m_instance, &numDevices, &physDevices[0]) != VK_SUCCESS)
	{
		std::cout << "Error during Vulkan Device enumeration!" << std::endl;
		return false;
	}

	// now that we have our devices, let's check which one supports rendering
	for (VkPhysicalDevice& physDevice : physDevices)
	{
		unsigned int numQueueFamilies;
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physDevice, &properties);

		if (m_bDebugInstance)
		{
			// need OutputDebugString on windows
			std::cout << "Device: " << properties.deviceName << std::endl;
		}
		// get extension properties for this device
		uint32_t numExtensionProps;
		if (vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &numExtensionProps, nullptr) != VK_SUCCESS)
		{
			std::cout << "Failed to eumerate extensions for device" << std::endl;
			continue;
		}

		// no extensions, however we need support for swapchain rendering, so continue to the next device
		if (numExtensionProps == 0)
			continue;

		std::vector<VkExtensionProperties> deviceExtensions(numExtensionProps);
		if (vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &numExtensionProps, &deviceExtensions[0]) != VK_SUCCESS)
		{
			std::cout << "Failed to eumerate extensions for device" << std::endl;
			continue;
		}

		bool bSupportsSwapchainRendering = false;
		bool bSupportsMaintainance1 = false;
		for (VkExtensionProperties& property : deviceExtensions)
		{
			if (strcmp(property.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				bSupportsSwapchainRendering = true;
			}
			else if (strcmp(property.extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME) == 0)
			{
				bSupportsMaintainance1 = true;
			}
		}

		if (!(bSupportsSwapchainRendering && bSupportsMaintainance1))
			continue;

		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &numQueueFamilies, nullptr);

		std::vector <VkQueueFamilyProperties> familyProperties(numQueueFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &numQueueFamilies, &familyProperties[0]);

		uint32_t presentQueueIndex = 0xFFFFFFFF, graphicsQueueIndex = 0xFFFFFFFF;

		for (uint32_t familyIndex = 0; familyIndex < familyProperties.size(); ++familyIndex)
		{
			VkQueueFamilyProperties& queueFamilyProperty  = familyProperties[familyIndex];

			bool bSupportsGraphicsCommands = queueFamilyProperty.queueCount > 0 &&
					(queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT);

			bSupportsSwapchainRendering = false;

			VkBool32 vkbSupportsSwapchainRendering;
			if (vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, familyIndex, surface, &vkbSupportsSwapchainRendering) == VK_SUCCESS)
			{
				bSupportsSwapchainRendering = (vkbSupportsSwapchainRendering == VK_TRUE);
			}

			if (presentQueueIndex == 0xFFFFFFFF && bSupportsSwapchainRendering)
			{
				presentQueueIndex = familyIndex;
			}

			if (graphicsQueueIndex == 0xFFFFFFFF && bSupportsGraphicsCommands)
			{
				graphicsQueueIndex = familyIndex;
			}

			if (bSupportsGraphicsCommands && bSupportsSwapchainRendering)
			{
				presentQueueIndex = graphicsQueueIndex = familyIndex;
				break;
			}
		}

		if (presentQueueIndex != 0xFFFFFFFF && graphicsQueueIndex != 0xFFFFFFFF)
		{
			std::vector<float> queuePriorities = { 1.0f };

			std::vector <VkDeviceQueueCreateInfo> queueCreateInfo;

			VkDeviceQueueCreateInfo presentQueueCreateInfo =
			{
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				nullptr,
				0,
				presentQueueIndex,
				static_cast<uint32_t> (queuePriorities.size()),
				queuePriorities.data()
			};

			queueCreateInfo.push_back(presentQueueCreateInfo);

			if (presentQueueIndex != graphicsQueueIndex)
			{
				VkDeviceQueueCreateInfo graphicsQueueCreateInfo =
				{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					nullptr,
					0,
					graphicsQueueIndex,
					static_cast<uint32_t> (queuePriorities.size()),
					queuePriorities.data()
				};

				queueCreateInfo.push_back(graphicsQueueCreateInfo);
			}

			std::vector<const char*> extensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_KHR_MAINTENANCE1_EXTENSION_NAME
			};

			VkDeviceCreateInfo deviceCreateInfo =
			{
				VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(queueCreateInfo.size()),
				queueCreateInfo.data(),
				0,
				nullptr,
				static_cast<uint32_t>(extensions.size()),
				extensions.data(),
				nullptr
			};

			if (vkCreateDevice(physDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
			{
				std::cout << "Failure during device creation" << std::endl;
				continue;
			}

			m_physicalDevice = physDevice;
			m_graphicsQueueIndex = graphicsQueueIndex;
			m_presentQueueIndex = presentQueueIndex;
			m_graphicsCommandPool = VK_NULL_HANDLE;

			bool bDeviceFunctionsLoaded = true;
			#define VK_DEVICE_FUNCTION( fun ) \
			if (!(fun = (PFN_##fun)vkGetDeviceProcAddr(m_device, #fun))) { \
				std::cout << "Could not load global function: " << #fun << "!" << std::endl; \
				bDeviceFunctionsLoaded = false; \
			}
			#include "VulkanFunctions.inl"
			#undef VK_DEVICE_FUNCTION

			if (!bDeviceFunctionsLoaded)
			{
				return false;
			}

			vkGetDeviceQueue(m_device, graphicsQueueIndex, 0, &m_graphicsQueue);
			vkGetDeviceQueue(m_device, presentQueueIndex, 0, &m_presentQueue);

			// create command pools and buffers
			VkCommandPoolCreateInfo commandPoolCreateInfo =
			{
				VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				nullptr,
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				m_graphicsQueueIndex
			};

			if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_graphicsCommandPool) != VK_SUCCESS)
			{
				std::cout << "Could not create graphics command pool" << std::endl;
			}
		}
	}

	return true;
}

CVulkanDevice::CVulkanDevice(GameWindow& win, bool bDebugContext)
	: m_instance(VK_NULL_HANDLE)
	, m_debugHandle(VK_NULL_HANDLE)
	, m_device(VK_NULL_HANDLE)
	, m_graphicsCommandPool(VK_NULL_HANDLE)
	, m_graphicsQueueIndex(0)
	, m_physicalDevice(VK_NULL_HANDLE)
	, m_presentQueue(VK_NULL_HANDLE)
	, m_presentQueueIndex(0)
	, m_graphicsQueue(VK_NULL_HANDLE)
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
		return;

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
		return;


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
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

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
		"Space Pirates",
		VK_MAKE_VERSION(1, 0, 0),
		"Mini Engine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_1
	};

	std::vector <const char*> enabledLayers;
	if (m_bDebugInstance)
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

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
	if (vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS)
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
//			VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
//          VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
//			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			0,
			vulkanDebugCallback,
			nullptr
		};

		vkCreateDebugReportCallbackEXT(m_instance, &debugCreateInfo, nullptr, &m_debugHandle);
	}

	// create swapchain and device now
	auto swapchain = std::make_unique <CVulkanSwapchain> (win);
	win.assignSwapchain(std::move(swapchain));
}

CVulkanDevice::~CVulkanDevice()
{
	vkDeviceWaitIdle(m_device);

	// first, delete the device if it exists
	if (m_graphicsCommandPool)
	{
		vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
	}

	// clear the heaps before destroying the device
	m_memoryHeaps.clear();

	if (m_device)
	{
		vkDestroyDevice(m_device, nullptr);
	}

	if (m_bDebugInstance)
	{
		vkDestroyDebugReportCallbackEXT(m_instance, m_debugHandle, nullptr);
	}

	vkDestroyInstance(m_instance, nullptr);

	CloseDll(m_librarymodule);
}

bool CVulkanDevice::getSwapchainCreationParameters(VkSurfaceKHR windowSurface, VkSwapchainKHR oldSwapchain, VkSwapchainCreateInfoKHR& swapchainCreateInfo)
{
	// first, query some capabilities for current device surface extensions
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, windowSurface, &surfaceCapabilities) != VK_SUCCESS)
	{
		std::cout << "Error during surface capability enumeration!" << std::endl;
		return false;
	}

	VkBool32 vkbSupportsSwapchainRendering = VK_FALSE;
	if (vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_graphicsQueueIndex, windowSurface, &vkbSupportsSwapchainRendering) != VK_SUCCESS ||
		(vkbSupportsSwapchainRendering == VK_FALSE))
	{
		std::cout << "Window surface not supported by device graphics queue!" << std::endl;
		return false;
	}

	vkbSupportsSwapchainRendering = VK_FALSE;
	if (vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_presentQueueIndex, windowSurface, &vkbSupportsSwapchainRendering) != VK_SUCCESS ||
		(vkbSupportsSwapchainRendering == VK_FALSE))
	{
		std::cout << "Window surface not supported by device present queue!" << std::endl;
		return false;
	}

	uint32_t surfaceFormatCount;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, windowSurface, &surfaceFormatCount, nullptr) != VK_SUCCESS)
	{
		std::cout << "Error during format enumeration!" << std::endl;
		return false;
	}

	std::vector <VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);

	if (vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, windowSurface, &surfaceFormatCount, &surfaceFormats[0]) != VK_SUCCESS)
	{
		std::cout << "Error during format enumeration!" << std::endl;
		return false;
	}

	uint32_t presentModeCount;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, windowSurface, &presentModeCount, nullptr) != VK_SUCCESS)
	{
		std::cout << "Error during present mode enumeration!" << std::endl;
		return false;
	}

	std::vector <VkPresentModeKHR> presentModes(presentModeCount);

	if (vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, windowSurface, &presentModeCount, &presentModes[0]) != VK_SUCCESS)
	{
		std::cout << "Error during present mode enumeration!" << std::endl;
		return false;
	}

	// hardcoded, should be option instead
	uint32_t finalImageCount = 2;
	finalImageCount = std::max(std::min(surfaceCapabilities.maxImageCount, finalImageCount), surfaceCapabilities.minImageCount);

	bool bFormatAllowed = false;

	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		bFormatAllowed = true;
	}

	for (VkSurfaceFormatKHR surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format ==VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
		{
			bFormatAllowed = true;
			break;
		}
	}

	if (!bFormatAllowed)
	{
		std::cout << "No suitable swapchain format found!" << std::endl;
		return false;
	}

	VkSurfaceFormatKHR finalFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
	VkExtent2D finalExtent = surfaceCapabilities.currentExtent;

	// use an arbitrary extent in case of invalid values
	if (finalExtent.width == ~0x0u && finalExtent.height == ~0x0u)
	{
		finalExtent.width = 800;
		finalExtent.width = 600;
	}

	// make sure to clamp between minimum and maximum
	finalExtent.width = std::min(finalExtent.width, surfaceCapabilities.maxImageExtent.width);
	finalExtent.width = std::max(finalExtent.width, surfaceCapabilities.minImageExtent.width);

	finalExtent.height = std::min(finalExtent.height, surfaceCapabilities.maxImageExtent.height);;
	finalExtent.height = std::max(finalExtent.height, surfaceCapabilities.minImageExtent.height);

	swapchainCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		windowSurface,
		finalImageCount,
		finalFormat.format,
		finalFormat.colorSpace,
		finalExtent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_PRESENT_MODE_FIFO_KHR,
		VK_TRUE,
		oldSwapchain
	};

	return true;
}

bool CVulkanDevice::allocateMemory(SMemoryChunk** chunk, size_t& offset, VkMemoryRequirements& requirements, bool bMappable)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;

	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	VkMemoryPropertyFlags memoryFlags = 0;

	if (bMappable)
	{
		memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	}

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		uint32_t bitValue = (1 << i);
		if (((requirements.memoryTypeBits & bitValue) != 0) &&
			((memoryProperties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags))
		{
			// first, check if we have already allocated a memory chunk from this pool
			if (m_memoryHeaps.find(i) != m_memoryHeaps.end())
			{
				SMemoryHeap* memoryHeap = m_memoryHeaps[i].get();

				for (SMemoryChunk::Ptr& memoryChunk : memoryHeap->m_chunks)
				{
					if (memoryChunk->allocateBlock(requirements.size, requirements.alignment, offset))
					{
						*chunk = memoryChunk.get();
						return true;
					}
				}
			}

			// chunks are allocations of 128 MB each
			const size_t chunkSize = std::max(static_cast <VkDeviceSize> (128 * 1024 * 1024), requirements.size);

			// we failed to find an empty block, proceed to allocate a new chunk of appropriate size
			VkDeviceMemory memory;
			VkMemoryAllocateInfo allocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				chunkSize,
				i
			};

			VkResult result = vkAllocateMemory(m_device, &allocateInfo, nullptr, &memory);
			if (result != VK_SUCCESS)
			{
				if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				{
					// continue, it may be possible to find another kind of memory to accomodate the request
					continue;
				}

				std::cout << "Failed to allocate memory" << std::endl;
				return false;
			}

			if (m_memoryHeaps.find(i) == m_memoryHeaps.end())
			{
				SMemoryHeap::Ptr newInfo = std::make_unique <SMemoryHeap> ();
				m_memoryHeaps.insert(std::make_pair(i, std::move(newInfo)));
			}

			// create and append a new chunk in the memory type array
			std::vector <SMemoryChunk::Ptr>& container = m_memoryHeaps[i]->m_chunks;

			// finally, move the new chunk in the container for this type of memory
			container.push_back(std::make_unique <SMemoryChunk> (memory, chunkSize));
			SMemoryChunk::Ptr& newChunk = container.back();

			//container.push_back(std::move(newChunk));
			if (newChunk->allocateBlock(requirements.size, requirements.alignment, offset))
			{
				*chunk = newChunk.get();
				return true;
			}
		}
	}

	return false;
}

bool CVulkanDevice::getSupportsImage(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags)
{
	VkImageFormatProperties imageFormatProperties;
	VkResult result = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice, format, type, tiling, usage, flags, &imageFormatProperties);

	if (result == VK_SUCCESS)
	{
		return true;
	}
	else if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
	{
		std::cout << "Error, Image format not supported" << std::endl;
	}
	else {
		std::cout << "Error during image format support query" << std::endl;
	}

	return false;
}

std::unique_ptr<IGPUBuffer> CVulkanDevice::createGPUBuffer(size_t size, uint32_t usage)
{
	return std::make_unique <CVulkanBuffer> (size, usage);
}

std::unique_ptr<IPipeline> CVulkanDevice::createPipeline(SPipelineParams& params)
{
	return std::make_unique <CVulkanPipeline>(params);
}

std::unique_ptr<ITexture> CVulkanDevice::createTexture(ITexture::EFormat format, uint32_t usage, uint32_t width, uint32_t height, bool bMipmapped)
{
	return std::make_unique <CVulkanTexture> (format, usage, width, height, bMipmapped);
}

void CVulkanDevice::finishJobs()
{
	vkDeviceWaitIdle(m_device);
}

std::unique_ptr<IRenderPass> CVulkanDevice::createRenderPass(SRenderPassParams& params)
{
	return std::make_unique <CVulkanRenderPass> (params);
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

	for (SMemoryBlock& block : m_blocks)
	{
		if (block.m_bUsed)
		{
			std::cout << "Deleting Memory Block at offset " << block.m_offset << " which is still used!" << std::endl;
		}
	}

	if (m_memory)
	{
		device.vkFreeMemory(device, m_memory, nullptr);
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

					auto iter = m_blocks.begin();
					std::advance(iter, index + 1);
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
			auto mergeStartIter = m_blocks.begin();
			std::advance(mergeStartIter, index);
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

VkShaderStageFlags stageFlagsToVulkanFlags(uint32_t stages)
{
	VkShaderStageFlags flags = 0;

	if (stages & eVertexStage)
	{
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (stages & eFragmentStage)
	{
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	return flags;
}

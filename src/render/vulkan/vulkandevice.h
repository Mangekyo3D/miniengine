#pragma once
#include "../idevice.h"
#include "vulkan/vulkan.h"
#include <memory>
#include <map>
#include <vector>

class IGPUBuffer;
class CVulkanSwapchain;
class CViewport;
struct SFrame;

// block returned to vulkan resources from the memory manager for buffers and textures
struct SMemoryBlock
{
	// offset of buffer in memory region
	size_t     m_offset;
	size_t     m_size;
	// stores whether this block is freed or used
	bool       m_bUsed;
};

struct SMemoryChunk
{
	typedef std::unique_ptr <SMemoryChunk> Ptr;

	SMemoryChunk(VkDeviceMemory memory, const size_t size);
	~SMemoryChunk();
	bool allocateBlock(size_t size, size_t alignment, size_t& offset);
	void freeBlock(size_t offset);

	// memory object for this memory chunk
	VkDeviceMemory m_memory;
	// total size of the chunk
	size_t           m_size;

	std::vector <SMemoryBlock> m_blocks;
};

struct SMemoryHeap
{
	typedef std::unique_ptr <SMemoryHeap> Ptr;

	std::vector <SMemoryChunk::Ptr> m_chunks;
};

class CVulkanDevice : public IDevice
{
public:
	CVulkanDevice(GameWindow& win, bool bDebugContext);
	~CVulkanDevice() override;
	static CVulkanDevice& get();

	virtual std::unique_ptr<ICommandBuffer> beginFrame(ISwapchain& currentSwapchain) override;
	virtual std::unique_ptr<IRenderPass> createRenderPass(SRenderPassParams&) override;
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size, uint32_t usage) override;
	virtual std::unique_ptr<IPipeline> createPipeline(SPipelineParams& params) override;
	virtual std::unique_ptr<ITexture> createTexture(ITexture::EFormat format, uint32_t usage, uint32_t width, uint32_t height, bool bMipmapped) override;
	virtual void finishJobs() override;

	VkPhysicalDevice getPhysicalDevice();

	// Creates a device compatible with rendering to a surface. Note, this call will only create a device once
	bool ensureDevice(VkSurfaceKHR surface);
	// conversion to instance and device for convenience - allows us to pass the device around in functions
	operator VkInstance () { return m_instance;}
	operator VkDevice () { return m_device;}

	bool getSwapchainCreationParameters(VkSurfaceKHR windowSurface, VkSwapchainKHR oldSwapchain, VkSwapchainCreateInfoKHR& swapchainCreateInfo);

	bool allocateMemory(SMemoryChunk** chunk, size_t& offset, VkMemoryRequirements& requirements, bool bMappable = true);

	bool getSupportsImage(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);

	VkCommandPool getGraphicsCommandPool(int) { return m_graphicsCommandPool; }

	uint32_t getGraphicsQueueIndex() { return m_graphicsQueueIndex; }
	uint32_t getPresentQueueIndex() { return m_presentQueueIndex; }
	VkQueue getGraphicsQueue() { return m_graphicsQueue; }
	VkQueue getPresentQueue() { return m_presentQueue; }

#if !defined(VK_INSTANCE_DEBUG_FUNCTION)
#define VK_INSTANCE_DEBUG_FUNCTION( fun) PFN_##fun fun = nullptr;
#endif

#if !defined(VK_INSTANCE_FUNCTION)
#define VK_INSTANCE_FUNCTION( fun) PFN_##fun fun = nullptr;
#endif

#if !defined(VK_DEVICE_FUNCTION)
#define VK_DEVICE_FUNCTION( fun) PFN_##fun fun = nullptr;
#endif

#include "VulkanFunctions.inl"

#undef VK_INSTANCE_DEBUG_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

private:
	void createRenderPasses();
	void createPipelines();
	void createMeshes();

	bool           m_bDebugInstance;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	HMODULE        m_librarymodule;
#elif defined VK_USE_PLATFORM_XLIB_KHR
	void*          m_librarymodule;
#endif

	VkInstance     m_instance;

	VkDebugReportCallbackEXT m_debugHandle;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	uint32_t m_graphicsQueueIndex;
	uint32_t m_presentQueueIndex;

	VkCommandPool m_graphicsCommandPool;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	// memory allocator specific
	std::map <uint32_t, SMemoryHeap::Ptr> m_memoryHeaps;

	static CVulkanDevice* s_device;
};

VkShaderStageFlags stageFlagsToVulkanFlags(uint32_t stages);

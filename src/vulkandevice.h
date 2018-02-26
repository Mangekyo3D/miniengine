#pragma once

#define NOMINMAX

#include "device.h"
#include "vulkan/vulkan.h"
#include <windows.h>
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
	// push the block for deletion after the current frame has finished executing
	void delayedFreeBlock(size_t offset);

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
	CVulkanDevice();
	~CVulkanDevice();
	static CVulkanDevice& get();

	virtual void initialize(GameWindow& win, bool bDebugContext) override;
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size) override;


	void initialize(bool bDebugInstance);
	VkDevice getDevice();
	VkPhysicalDevice getPhysicalDevice();

	// Creates a device compatible with rendering to a surface. Note, this call will only create a device once
	void createDevice(VkSurfaceKHR surface);
	operator VkInstance () { return m_instance;}

	VkRenderPass getRenderPass() { return m_renderPass; }

	void renderFrame(CVulkanSwapchain& swapchain, CViewport& viewport);

	bool getSwapchainCreationParameters(VkSurfaceKHR windowSurface, VkSwapchainKHR oldSwapchain, VkSwapchainCreateInfoKHR& swapchainCreateInfo);
	void beginFrame(SFrame& frame);
	void submitFrame(CVulkanSwapchain& swapchain, SFrame& frame);

	bool allocateMemory(SMemoryChunk** chunk, size_t& offset, VkMemoryRequirements& requirements, bool bMappable = true);

	bool getSupportsImage(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);

	VkCommandPool getGraphicsCommandPool(int nFrame) { return m_graphicsCommandPool; }
	VkDescriptorSetLayout& getScenePassDescriptorSetLayout() { return m_descriptorSetLayout; }

#if !defined(VK_INSTANCE_DEBUG_FUNCTION)
#define VK_INSTANCE_DEBUG_FUNCTION( fun) PFN_##fun fun;
#endif

#if !defined(VK_INSTANCE_FUNCTION)
#define VK_INSTANCE_FUNCTION( fun) PFN_##fun fun;
#endif

#if !defined(VK_GLOBAL_FUNCTION)
#define VK_GLOBAL_FUNCTION( fun) PFN_##fun fun;
#endif

#if !defined(VK_DEVICE_FUNCTION)
#define VK_DEVICE_FUNCTION( fun) PFN_##fun fun;
#endif

#include "VulkanFunctions.inl"

#undef VK_INSTANCE_DEBUG_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_DEVICE_FUNCTION

private:
	void createRenderPasses();
	void createPipelines();
	void createMeshes();

	bool           m_bDebugInstance;
	HMODULE        m_librarymodule;
	VkInstance     m_instance;

	VkDebugReportCallbackEXT m_debugHandle;

	// renderpass responsible for rendering to swapchains
	VkRenderPass m_renderPass;
	// pipeline for diffuse pass
	VkPipeline m_diffusePipeline;
	VkPipelineLayout m_pipelineLayout;

	VkDescriptorSetLayout m_descriptorSetLayout;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	uint32_t m_graphicsQueueIndex;
	uint32_t m_presentQueueIndex;

	VkCommandPool m_graphicsCommandPool;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	// memory allocator specific
	std::map <int, SMemoryHeap::Ptr> m_memoryHeaps;
};

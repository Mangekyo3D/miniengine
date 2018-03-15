#pragma once
#include "../iswapchain.h"
#include <vector>
#include "vulkan/vulkan.h"
class GameWindow;

struct SDeferredDeletionBlock
{
	struct SMemoryChunk* m_chunk;
	size_t m_offset;
};

// Frame that encapsulates data corresponding to state of one frame: command buffer, semaphores for synchronization and fences
struct SFrame
{
	SFrame();
	~SFrame();

	void cleanupDelayedBlocks();

	VkSemaphore m_swapchainImageAvailableSemaphore;
	VkSemaphore m_renderingFinishedSemaphore;

	VkImage m_swapchainImage;
	// image view for the swapchain image
	VkImageView m_imageView;
	VkFramebuffer m_framebuffer;

	// command buffer for this frame
	VkCommandBuffer m_commandBuffer;
	// fence that denotes when frame command buffer has finished executing
	VkFence m_fence;

	// blocks that should be deleted once the frame has finished executing
	std::vector<SDeferredDeletionBlock> m_deletedBlocks;
};

class CVulkanSwapchain : public ISwapchain
{
	public:
		CVulkanSwapchain(GameWindow& win);
		~CVulkanSwapchain();
		void swapBuffers() override;

		void recreate();
		void cleanup();

		SFrame& getCurrentFrame();

		operator VkSwapchainKHR& () { return m_swapchain; }
		VkExtent2D& getExtent() { return m_extent; }
		static uint8_t getNumberOfVirtualFrames() { return 2; }
		uint32_t getCurrentFrameImageIndex() { return m_currentSwapchainImage; }

	private:
		VkSurfaceKHR m_surface;
		VkExtent2D m_extent;

		uint32_t m_currentSwapchainImage;
		uint8_t m_currentFrameIndex;

		VkSwapchainKHR m_swapchain;
		uint32_t m_numSwapchainImages;

		std::vector <SFrame> m_frames;
		// semaphores for frame acquisition. Those are passed to SFrames to synchronize rendering
		std::vector <VkSemaphore> m_frameAcquireSemaphores;

		uint32_t m_availableSemaphores;
		VkSemaphore m_nextAvailableSemaphore;

		// one image per frame
		std::vector <VkImage> m_swapchainImages;
//		std::vector <VkFramebuffer> m_framebuffers;
		std::vector <VkImageView> m_swapchainImageviews;
};

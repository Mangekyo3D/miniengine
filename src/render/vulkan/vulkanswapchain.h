#pragma once
#include "../iswapchain.h"
#include <vector>
#include "vulkan/vulkan.h"
#include <memory>
#include <optional>

class GameWindow;
class CVulkanBuffer;
class CDescriptorPool;

// Frame that encapsulates data corresponding to state of one frame: command buffer, semaphores for synchronization and fences
struct SFrame
{
	SFrame();
	SFrame(const SFrame&) = delete;
	SFrame& operator = (const SFrame&) = delete;
	~SFrame();

	void cleanupOrphanedData();
	void orphanBuffer(std::unique_ptr<CVulkanBuffer> buffer);
	void orphanDescriptorPool(std::optional<CDescriptorPool>&& pool);

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
	std::vector<std::unique_ptr <CVulkanBuffer> > m_orphanedBuffers;
	std::vector<CDescriptorPool> m_orphanedPools;
};

class CVulkanSwapchain : public ISwapchain
{
	public:
		CVulkanSwapchain(GameWindow& win);
		~CVulkanSwapchain() override;
		void swapBuffers() override;

		void recreate();
		void cleanup();
		void onResize(uint32_t newWidth, uint32_t newHeight) override;

		SFrame& getNextFrame();

		operator VkSwapchainKHR& () { return m_swapchain; }
		VkExtent2D& getExtent() { return m_extent; }
		static uint8_t getNumberOfVirtualFrames() { return 2; }

	private:
		VkSurfaceKHR m_surface;
		VkExtent2D m_extent;

		uint32_t m_currentSwapchainImage;
		uint8_t m_currentFrameIndex;

		VkSwapchainKHR m_swapchain;
		uint32_t m_numSwapchainImages;

		std::vector <std::unique_ptr <SFrame> > m_frames;
		// semaphores for frame acquisition. Those are passed to SFrames to synchronize rendering
		std::vector <VkSemaphore> m_frameAcquireSemaphores;

		uint32_t m_availableSemaphores;
		VkSemaphore m_nextAvailableSemaphore;

		// one image per frame
		std::vector <VkImage> m_swapchainImages;
//		std::vector <VkFramebuffer> m_framebuffers;
		std::vector <VkImageView> m_swapchainImageviews;
};

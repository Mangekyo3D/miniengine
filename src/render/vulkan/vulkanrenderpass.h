#pragma once
#include "../irenderpass.h"
#include <vulkan/vulkan.h>

class CVulkanRenderPass : public IRenderPass
{
	public:
		CVulkanRenderPass();
		~CVulkanRenderPass();
		// setup the renderpass with inputs and outputs.
		virtual void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);
		VkRenderPass getID() { return m_renderPass; }
		VkFramebuffer getFramebuffer() { return m_framebuffer; }
		bool isSwapchainPass() const { return bIsSwapchainPass; }
		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }

	private:
		// setup framebuffers
		void setupFrameBuffer(ITexture** outputs = nullptr, uint32_t numOutputs = 0, ITexture* depthOut = 0);
		void ensureRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);

		uint32_t     m_width;
		uint32_t     m_height;
		VkRenderPass m_renderPass;
		VkFramebuffer m_framebuffer;
		bool bIsSwapchainPass;
};

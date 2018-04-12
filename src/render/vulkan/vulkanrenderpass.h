#pragma once
#include "../irenderpass.h"
#include <vulkan/vulkan.h>
#include <memory>

class CVulkanDescriptorSet;

class CVulkanRenderPass : public IRenderPass
{
	public:
		CVulkanRenderPass(SRenderPassParams& params);
		~CVulkanRenderPass();
		// setup the renderpass with inputs and outputs.
		virtual void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);
		VkFramebuffer getFramebuffer() { return m_framebuffer; }
		bool isSwapchainPass() const { return bIsSwapchainPass; }
		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }
		operator VkRenderPass () {return m_renderPass; }
		CVulkanDescriptorSet* getDescriptorSet() { return m_descriptorSet.get(); }
		bool is3DPass() {return m_b3DPass; }

	private:
		// setup framebuffers
		void setupFrameBuffer(ITexture** outputs = nullptr, uint32_t numOutputs = 0, ITexture* depthOut = 0);
		void ensureRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);

		uint32_t     m_width;
		uint32_t     m_height;
		VkRenderPass m_renderPass;
		VkFramebuffer m_framebuffer;
		bool bIsSwapchainPass;
		std::unique_ptr <CVulkanDescriptorSet> m_descriptorSet;
		bool m_b3DPass;
};

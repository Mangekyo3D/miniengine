#pragma once
#include "../icommandbuffer.h"
#include <vulkan/vulkan.h>
#include <memory>

class CVulkanDevice;
class CVulkanSwapchain;
class CVulkanBuffer;
struct SFrame;
class CDescriptorPool;
class ISwapchain;
class CVulkanPipeline;

class CVulkanCommandBuffer : public ICommandBuffer
{
	public:
		CVulkanCommandBuffer(ISwapchain& swapchain);
		~CVulkanCommandBuffer() override;

		virtual IGPUBuffer& createStreamingBuffer(size_t size) override;
		virtual void copyBufferToTex(ITexture* tex, size_t offset,
									 uint32_t width, uint32_t height, uint8_t miplevel) override;
		virtual void bindPipeline(IPipeline* pipeline, size_t numRequiredDescriptors) override;

		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer = nullptr,  IGPUBuffer* indexBuffer = nullptr, bool bShortIndex = true) override;

		virtual void drawIndexedInstanced(EPrimitiveType type, size_t numIndices, size_t offset, size_t numInstances) override;
		virtual void drawArrays(EPrimitiveType type, uint32_t start, uint32_t count) override;

		virtual void bindGlobalRenderPassDescriptors(size_t numBindings, SDescriptorSource*) override;
		virtual void bindGlobalPipelineDescriptors(size_t numBindings, SDescriptorSource*) override;
		virtual void bindPerDrawDescriptors(size_t numBindings, SDescriptorSource*) override;

		virtual IDevice& getDevice() override;

	protected:
		virtual void beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth) override;
		virtual void endRenderPass() override;
		VkDescriptorSet updateDescriptorsGeneric(CDescriptorPool* pool, size_t numBindings, SDescriptorSource* sources);

		CVulkanDevice* m_device;
		std::unique_ptr <CVulkanBuffer> m_streamingBuffer;
		std::unique_ptr <CDescriptorPool> m_renderpassGlobalPool;
		std::unique_ptr <CDescriptorPool> m_pipelineGlobalPool;
		std::unique_ptr <CDescriptorPool> m_pipelinePerDrawPool;
		CVulkanSwapchain* m_swapchain;
		VkPipelineLayout m_currentPipelineLayout;
		SFrame* m_frame;
		VkCommandBuffer m_cmd;
		uint32_t m_pipelineSetIndex;
		uint32_t m_perDrawSetIndex;

		VkDescriptorSet m_renderPassGlobalSet;
		VkDescriptorSet m_pipelineGlobalSet;
};

#pragma once
#include "../icommandbuffer.h"
#include <vulkan/vulkan.h>
#include <memory>

class CVulkanDevice;
class CVulkanSwapchain;
class CVulkanBuffer;
struct SFrame;
struct SDescriptorPool;
class ISwapchain;

class CVulkanCommandBuffer : public ICommandBuffer
{
	public:
		CVulkanCommandBuffer(ISwapchain& swapchain);
		~CVulkanCommandBuffer();

		virtual IGPUBuffer& createStreamingBuffer(size_t size) override;
		virtual void copyBufferToTex(ITexture* tex, size_t offset,
									 uint16_t width, uint16_t height, uint8_t miplevel) override;
		virtual void bindPipeline(IPipeline* pipeline, size_t numRequiredDescriptors) override;

		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer = nullptr,  IGPUBuffer* indexBuffer = nullptr, bool bShortIndex = true) override;

		virtual void drawIndexedInstanced(EPrimitiveType type, size_t numIndices, size_t offset, size_t numInstances) override;
		virtual void drawArrays(EPrimitiveType type, uint32_t start, uint32_t count) override;

		virtual void bindGlobalDescriptors(size_t numBindings, SDescriptorSource*);
		virtual void bindPerDrawDescriptors(size_t numBindings, SDescriptorSource*);

		virtual IDevice& getDevice() override;

	protected:
		virtual void beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth) override;
		virtual void endRenderPass() override;

		CVulkanDevice* m_device;
		std::unique_ptr <CVulkanBuffer> m_streamingBuffer;
		std::unique_ptr <SDescriptorPool> m_globalPool;
		std::unique_ptr <SDescriptorPool> m_perDrawPool;
		CVulkanSwapchain* m_swapchain;
		SFrame* m_frame;
		VkCommandBuffer m_cmd;
};

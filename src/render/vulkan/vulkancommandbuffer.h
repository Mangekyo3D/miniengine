#pragma once
#include "../icommandbuffer.h"
#include <vulkan/vulkan.h>

class CVulkanDevice;
class CVulkanSwapchain;
struct SFrame;
class ISwapchain;

class CVulkanCommandBuffer : public ICommandBuffer
{
	public:
		CVulkanCommandBuffer(ISwapchain& swapchain);
		~CVulkanCommandBuffer();

		virtual void setStreamingBuffer(IGPUBuffer* buf) override;
		virtual void copyBufferToTex(ITexture* tex, size_t offset,
									 uint16_t width, uint16_t height, uint8_t miplevel) override;
		virtual void bindPipeline(IPipeline* pipeline) override;

		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer = nullptr,  IGPUBuffer* indexBuffer = nullptr, bool bShortIndex = true) override;

		virtual void drawIndexedInstanced(EPrimitiveType type, size_t numIndices, size_t offset, size_t numInstances) override;
		virtual void drawArrays(EPrimitiveType type, uint32_t start, uint32_t count) override;

		virtual IDevice& getDevice() override;

	protected:
		virtual void beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth) override;
		virtual void endRenderPass() override;

		CVulkanDevice* m_device;
		CVulkanSwapchain* m_swapchain;
		SFrame* m_frame;
		VkCommandBuffer m_cmd;
};

#pragma once
#include "../icommandbuffer.h"
#include <stdint.h>
#include <memory>

class COpenGLDevice;
class COpenGLPipeline;
class COpenGLBuffer;
class COpenGLVertexDescriptorInterface;

class COpenGLCommandBuffer :public ICommandBuffer
{
	public:
		COpenGLCommandBuffer(uint32_t swapchainWidth, uint32_t swapchainHeight);
		~COpenGLCommandBuffer() override;

		IGPUBuffer& createStreamingBuffer(size_t size) override;

		virtual void copyBufferToTex(ITexture* tex, size_t offset, uint32_t width, uint32_t height, uint8_t miplevel) override;

		virtual void bindPipeline(IPipeline* pipeline, size_t numRequiredDescriptors) override;

		virtual void bindGlobalRenderPassDescriptors(size_t numBindings, SDescriptorSource*) override;
		virtual void bindGlobalPipelineDescriptors(size_t numBindings, SDescriptorSource* sources) override;
		virtual void bindPerDrawDescriptors(size_t numBindings, SDescriptorSource* sources) override;
		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer, IGPUBuffer* indexBuffer, bool bShortIndices) override;

        virtual void drawArrays(uint32_t start, uint32_t count) override;

        virtual void drawIndexedInstanced(size_t numIndices, size_t offset, size_t numInstances) override;

		virtual IDevice& getDevice() override;

	protected:
		virtual void beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth) override;

		virtual void endRenderPass() override;

	private:
		uint32_t m_swapchainWidth, m_swapchainHeight;
		COpenGLDevice* m_device;
		COpenGLPipeline* m_currentPipeline;
		COpenGLVertexDescriptorInterface* m_currentVertexDescriptor;
		bool m_bShortIndices;
		bool m_bPrimitiveRestart;
		std::unique_ptr <COpenGLBuffer> m_streamingBuffer;
		uint32_t m_numGlobalPipelineBuffers;
		uint32_t m_numGlobalPipelineTextures;
		uint32_t m_numRenderpassBuffers;
		uint32_t m_numRenderpassTextures;
};

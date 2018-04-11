#pragma once
#include <stdint.h>
#include <stddef.h>

class ITexture;
class IGPUBuffer;
class IPipeline;
class IRenderPass;
class ICommandBuffer;
class IDevice;

struct SDescriptorSource
{
	SDescriptorSource(IGPUBuffer* buf)
	{
		data.buffer = buf;
		type = eBuffer;
	}

	SDescriptorSource(ITexture* tex)
	{
		data.texture = tex;
		type = eTexture;
	}

	union{
		IGPUBuffer* buffer;
		ITexture*   texture;
	} data;

	enum EDescriptorSource
	{
		eBuffer,
		eTexture
	} type;
};

class ICommandBuffer
{
	public:
		class CScopedRenderPass
		{
			public:
				CScopedRenderPass(ICommandBuffer& cmd, IRenderPass& renderpass, const float* vClearColor = nullptr, const float* clearDepth = nullptr);
				~CScopedRenderPass();

			private:
				IRenderPass* m_renderpass;
				ICommandBuffer* m_cmd;
		};

		enum class EPrimitiveType
		{
			eTriangles,
			eTriangleStrip
		};

		virtual ~ICommandBuffer() {}

		virtual IGPUBuffer& createStreamingBuffer(size_t size) = 0;
		virtual void copyBufferToTex(ITexture* tex, size_t offset,
									 uint16_t width, uint16_t height, uint8_t miplevel) = 0;
		virtual void bindPipeline(IPipeline* pipeline, size_t numRequiredDescriptors) = 0;
		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer = nullptr, IGPUBuffer* indexBuffer = nullptr, bool bShortIndex = true) = 0;
		virtual void drawIndexedInstanced(EPrimitiveType type, size_t numIndices, size_t offset, size_t numInstances) = 0;
		virtual void drawArrays(EPrimitiveType type, uint32_t start, uint32_t count) = 0;

		virtual void bindGlobalRenderPassDescriptors(size_t numBindings, SDescriptorSource*) = 0;
		virtual void bindGlobalPipelineDescriptors(size_t numBindings, SDescriptorSource*) = 0;
		virtual void bindPerDrawDescriptors(size_t numBindings, SDescriptorSource*) = 0;

		virtual IDevice& getDevice() = 0;

	protected:
		virtual void beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth) = 0;
		virtual void endRenderPass() = 0;
};

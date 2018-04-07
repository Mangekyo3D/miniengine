#pragma once
#include <stdint.h>

class ITexture;
class IGPUBuffer;
class IPipeline;
class IRenderPass;
class ICommandBuffer;
class IDevice;

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

		virtual void setStreamingBuffer(IGPUBuffer* buf) = 0;
		virtual void copyBufferToTex(ITexture* tex, size_t offset,
									 uint16_t width, uint16_t height, uint8_t miplevel) = 0;
		virtual void bindPipeline(IPipeline* pipeline) = 0;
		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* indexBuffer = nullptr, IGPUBuffer* instanceBuffer = nullptr) = 0;
		virtual void drawIndexedInstanced() {}
		virtual void drawArrays(EPrimitiveType type, uint32_t start, uint32_t end) = 0;
	//	device.glDrawElementsInstanced(meshPrimitiveToGLPrimitive(m_primType),static_cast <GLint> (m_numIndices),
	//								   (m_bShortIndices) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, nullptr,
	//								   static_cast <GLint> (m_instanceData.size()));

		virtual IDevice& getDevice() = 0;

	protected:
		virtual void beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth) = 0;
		virtual void endRenderPass() = 0;
};

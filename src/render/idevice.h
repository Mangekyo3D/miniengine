#pragma once
#include <memory>
#include <stdint.h>
#include <vector>
#include "itexture.h"

class IBatch;
class GameWindow;
class IGPUBuffer;
class IPipeline;

enum EPipelineFlags
{
	eDepthFuncGreater = 1L,
	eCullBackFace     = (1L << 1),
	ePrimitiveRestart = (1L << 2)
};

struct SPipelineParams
{
	SPipelineParams()
	{
		m_minDepth = 0.0f;
		m_maxDepth = 1.0f;
		m_flags = 0;
	}

	uint64_t m_flags;
	float m_minDepth, m_maxDepth;
};

enum EVertexFormat
{
	eFloat,
	e1010102int
};

struct SVertexAttribParams
{
	SVertexAttribParams()
	{
	}

	SVertexAttribParams(uint32_t ofs, EVertexFormat fmt,uint8_t comp)
		: offset(ofs)
		, format(fmt)
		, components(comp)
	{
	}

	uint32_t offset;
	EVertexFormat format;
	uint8_t components;
};

struct SVertexBinding
{
	SVertexBinding(size_t dataSize)
		: m_dataSize(dataSize)
	{
	}

	void addAttribute(uint32_t offset, EVertexFormat format,uint8_t components)
	{
		m_attributeParams.emplace_back(offset, format,components);
	}

	std::vector <SVertexAttribParams> m_attributeParams;
	size_t                              m_dataSize;
};

struct TextureStreamRequest
{
	TextureStreamRequest(ITexture* tex, const char* filename)
		: m_texture(tex)
		, m_filename(filename)
	{
	}

	size_t calculateSize() const;

	ITexture* m_texture;
	std::string m_filename;
};

class ICommandBuffer
{
	public:
		virtual ~ICommandBuffer() {}

		virtual void setStreamingBuffer(IGPUBuffer* buf) = 0;
		virtual void copyBufferToTex(ITexture* tex, size_t offset,
									 uint16_t width, uint16_t height, uint8_t miplevel) = 0;
		virtual void bindPipeline(IPipeline* pipeline) = 0;
		virtual void setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* indexBuffer, IGPUBuffer* instanceBuffer) = 0;
		virtual void drawIndexedInstanced() {}
		virtual void drawArrays() {}
	//	device.glDrawElementsInstanced(meshPrimitiveToGLPrimitive(m_primType),static_cast <GLint> (m_numIndices),
	//								   (m_bShortIndices) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, nullptr,
	//								   static_cast <GLint> (m_instanceData.size()));

};

class IDevice
{
public:
	IDevice()
		: m_texStreamBufferSize(0)
	{}
	IDevice(const IDevice&) = delete;
	virtual ~IDevice() {}
	virtual std::unique_ptr<ICommandBuffer> beginFrame() = 0;
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size) = 0;
	virtual std::unique_ptr<IPipeline> createPipeline(SPipelineParams& params, SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding,
													  const char* shaderName) = 0;
	virtual std::unique_ptr<ITexture> createTexture(ITexture::EFormat format, uint16_t width, uint16_t height, bool bMipmapped = false) = 0;

	void addTextureStreamRequest(TextureStreamRequest req);
	void flushPendingStreamRequests(ICommandBuffer&);

	static std::unique_ptr<IDevice> createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice);

protected:
	std::vector <TextureStreamRequest> m_textureRequests;
	size_t m_texStreamBufferSize;
};

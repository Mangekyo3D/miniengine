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

		virtual void copyBufferToTex(IGPUBuffer* buf, ITexture* tex, size_t offset,
									 uint16_t width, uint16_t height, uint8_t miplevel) = 0;
		virtual void bindPipeline(IPipeline* pipeline) = 0;
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
	virtual std::unique_ptr<IPipeline> createPipeline(SPipelineParams& params, const char* shaderName) = 0;
	virtual std::unique_ptr<ITexture> createTexture(ITexture::EFormat format, uint16_t width, uint16_t height, bool bMipmapped = false) = 0;

	void addTextureStreamRequest(TextureStreamRequest req);
	void flushPendingStreamRequests(ICommandBuffer*);

	static std::unique_ptr<IDevice> createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice);

protected:
	std::vector <TextureStreamRequest> m_textureRequests;
	size_t m_texStreamBufferSize;
};

#pragma once
#include <memory>
#include <stdint.h>
#include <vector>
#include "itexture.h"

class GameWindow;
class IPipeline;
class IRenderPass;
class ICommandBuffer;
class ISwapchain;
class IGPUBuffer;
struct SPipelineParams;
struct SVertexBinding;

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

class IDevice
{
public:
	IDevice()
		: m_texStreamBufferSize(0)
	{}
	IDevice(const IDevice&) = delete;
	virtual ~IDevice() {}
	virtual std::unique_ptr<ICommandBuffer> beginFrame(ISwapchain& swapchain) = 0;
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size, uint32_t usage) = 0;
	virtual std::unique_ptr<IRenderPass> createRenderPass() = 0;
	virtual std::unique_ptr<IPipeline> createPipeline(IRenderPass& renderpass, SPipelineParams& params, SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding,
													  const char* shaderName) = 0;
	virtual std::unique_ptr<ITexture> createTexture(ITexture::EFormat format, uint32_t usage, uint16_t width, uint16_t height, bool bMipmapped = false) = 0;

	void addTextureStreamRequest(TextureStreamRequest req);
	void flushPendingStreamRequests(ICommandBuffer&);

	static std::unique_ptr<IDevice> createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice);

protected:
	std::vector <TextureStreamRequest> m_textureRequests;
	size_t m_texStreamBufferSize;
};

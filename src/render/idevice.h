#pragma once
#include <memory>
#include "itexture.h"

class IBatch;
class GameWindow;
class IGPUBuffer;
class IPipeline;

struct SPipelineParams;

class IDevice
{
public:
	IDevice() {}
	IDevice(const IDevice&) = delete;
	virtual ~IDevice() {}
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size) = 0;
	virtual std::unique_ptr<IPipeline> createPipeline(SPipelineParams&) = 0;
	virtual std::unique_ptr<ITexture> createTexture(ITexture::EFormat format, uint16_t width, uint16_t height, bool bMipmapped = false) = 0;

	static IDevice& get() { return *s_device; }
	static std::unique_ptr<IDevice> createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice);

protected:
	static IDevice* s_device;
};

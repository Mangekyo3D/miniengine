#pragma once
#include <memory>

class IBatch;
class GameWindow;
class IGPUBuffer;

class IDevice
{
public:
	IDevice() {}
	IDevice(const IDevice&) = delete;
	virtual ~IDevice() {}
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size) = 0;

	static std::unique_ptr<IDevice> createDevice(GameWindow& win, bool bDebugContext, bool bVulkanContext);

protected:
	static IDevice* s_device;
};

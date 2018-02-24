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
	virtual void initialize(GameWindow& win, bool bDebugContext) = 0;
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size) = 0;

	static std::unique_ptr<IDevice> createDevice(bool bVulkanDevice);

protected:
	static IDevice* s_device;
};

#pragma once
#include <memory>

class IBatch;
class GameWindow;

class IDevice
{
public:
	IDevice() {}
	IDevice(const IDevice&) = delete;
	virtual ~IDevice() {}

	static std::unique_ptr<IDevice> createDevice(GameWindow &win, bool bDebugContext, bool bVulkanDevice);
	template <class T> static T& get() { return static_cast<T&> (*s_device); }

protected:
	static IDevice* s_device;
};

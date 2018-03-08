#pragma once
#include <memory>
class IDevice;
class GameWindow;

class CVulkanDeviceFactory
{
public:
	static std::unique_ptr<IDevice> createDevice(GameWindow& win, bool bDebugContext);
};

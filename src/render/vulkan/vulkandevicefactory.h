#pragma once
#include <memory>
#include "../device.h"

class CVulkanDeviceFactory
{
public:
	static std::unique_ptr<IDevice> createDevice();
};

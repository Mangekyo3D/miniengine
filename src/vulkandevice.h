#pragma once
#include "device.h"

class CVulkanDevice : public IDevice
{
public:
	CVulkanDevice(GameWindow& win, bool bDebugContext);
};

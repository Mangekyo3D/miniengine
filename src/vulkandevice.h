#pragma once
#include "device.h"

class IGPUBuffer;

class CVulkanDevice : public IDevice
{
public:
	CVulkanDevice(GameWindow& win, bool bDebugContext);

	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size);
};

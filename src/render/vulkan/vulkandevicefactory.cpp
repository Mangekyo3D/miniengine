#include "vulkandevicefactory.h"
#include "vulkandevice.h"

std::unique_ptr<IDevice> CVulkanDeviceFactory::createDevice(GameWindow& win, bool bDebugContext)
{
	return std::make_unique<CVulkanDevice> (win, bDebugContext);
}

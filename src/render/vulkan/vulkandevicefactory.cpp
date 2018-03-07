#include "vulkandevicefactory.h"
#include "vulkandevice.h"

std::unique_ptr<IDevice> CVulkanDeviceFactory::createDevice()
{
	return std::make_unique<CVulkanDevice> ();
}

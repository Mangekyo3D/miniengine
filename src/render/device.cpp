#include "opengl/opengldevice.h"
#if BUILD_WITH_VULKAN
#include "vulkan/vulkandevicefactory.h"
#endif

std::unique_ptr<IDevice> IDevice::createDevice(bool bVulkanDevice)
{
#if BUILD_WITH_VULKAN
	if (bVulkanDevice)
	{
		return CVulkanDeviceFactory::createDevice();
	}
	else
#endif
	{
		return std::make_unique<COpenGLDevice> ();
	}
}

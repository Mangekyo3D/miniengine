#include "opengldevice.h"
#if BUILD_WITH_VULKAN
#include "vulkandevice.h"
#endif

std::unique_ptr<IDevice> IDevice::createDevice(bool bVulkanDevice)
{
#if BUILD_WITH_VULKAN
	if (bVulkanDevice)
	{
		return std::make_unique<CVulkanDevice> ();
	}
	else
#endif
	{
		return std::make_unique<COpenGLDevice> ();
	}
}

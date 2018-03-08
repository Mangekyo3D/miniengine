#include "opengl/opengldevice.h"
#if BUILD_WITH_VULKAN
#include "vulkan/vulkandevicefactory.h"
#endif

std::unique_ptr<IDevice> IDevice::createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice)
{
#if BUILD_WITH_VULKAN
	if (bVulkanDevice)
	{
		return CVulkanDeviceFactory::createDevice(win, bDebugContext);
	}
	else
#endif
	{
		return std::make_unique<COpenGLDevice> (win, bDebugContext);
	}
}

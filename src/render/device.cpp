#include "opengl/opengldevice.h"
#include "vulkan/vulkandevicefactory.h"

std::unique_ptr<IDevice> IDevice::createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice)
{
	if (bVulkanDevice)
	{
		return CVulkanDeviceFactory::createDevice(win, bDebugContext);
	}
	else
	{
		return std::make_unique<COpenGLDevice> (win, bDebugContext);
	}
}

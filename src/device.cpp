#include "opengldevice.h"
#include "vulkandevice.h"

std::unique_ptr<IDevice> IDevice::createDevice(GameWindow& win, bool bDebugContext, bool bVulkanDevice)
{
	if (bVulkanDevice)
	{
		return std::make_unique<CVulkanDevice> (win, bDebugContext);
	}
	else
	{
		return std::make_unique<COpenGLDevice> (win, bDebugContext);
	}
}

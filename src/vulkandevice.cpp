#include "vulkandevice.h"
#include "gpubuffer.h"

CVulkanDevice::CVulkanDevice(GameWindow &win, bool bDebugContext)
{

}

std::unique_ptr<IGPUBuffer> CVulkanDevice::createGPUBuffer(size_t size)
{
	return nullptr;
}

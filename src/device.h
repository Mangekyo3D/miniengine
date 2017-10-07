#pragma once
#include <memory>

class IBatch;
class GameWindow;

class IDevice
{
public:
	virtual ~IDevice() {}
	virtual void drawBatch(IBatch& batch) = 0;
	virtual void clearFramebuffer(bool bDepth) = 0;
	virtual void setViewport(uint32_t width, uint32_t height) = 0;

	static std::unique_ptr<IDevice> createDevice(GameWindow &win, bool bDebugContext);
	template <class T> static T& get() { return static_cast<T&> (*s_device); }

protected:
	static IDevice* s_device;
};

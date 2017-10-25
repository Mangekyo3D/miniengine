#pragma once
#include <memory>

class IBatch;
class GameWindow;

class IDevice
{
public:
	virtual ~IDevice() {}

	static std::unique_ptr<IDevice> createDevice(GameWindow &win, bool bDebugContext);
	template <class T> static T& get() { return static_cast<T&> (*s_device); }

protected:
	static IDevice* s_device;
};

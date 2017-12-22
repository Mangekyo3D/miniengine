#pragma once
#include <memory>

class IAudioDevice
{
public:
	virtual ~IAudioDevice() {}

	static void initialize();
	static void shutdown();
	static IAudioDevice& get();

	virtual bool checkStatus() = 0;

protected:
	static std::unique_ptr <IAudioDevice> s_audioDevice;
	IAudioDevice() {}
	IAudioDevice(const IAudioDevice&) = delete;
};

class IAudioComponent
{
public:
	~IAudioComponent() {}

	virtual void playOnce() = 0;
	virtual void loop() = 0;

	virtual void setLocation() = 0;
};
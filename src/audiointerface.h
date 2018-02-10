#pragma once
#include <memory>
#include <string>

class IAudioResource;

class IAudioDevice
{
public:
	virtual ~IAudioDevice() {}

	static void initialize();
	static void shutdown();
	static IAudioDevice& get();

	virtual bool checkStatus() = 0;
	virtual std::unique_ptr <IAudioResource> createAudioResource(std::string& filename) = 0;

protected:
	static std::unique_ptr <IAudioDevice> s_audioDevice;
	IAudioDevice() {}
	IAudioDevice(const IAudioDevice&) = delete;
};

/**
 * @brief A sound resource is the sound data, stored in memory, sometimes with an
 *        API-specific way to store them internally */
class IAudioResource
{
public:
	IAudioResource(std::string& filename)
		: m_filename(filename)
	{}
	virtual ~IAudioResource() {}

protected:
	std::string m_filename;
};

/**
 * @brief An in game instance of an audio resource
 */
class IAudioInstance
{
public:
	~IAudioInstance() {}

	virtual void playOnce() = 0;
	virtual void loop() = 0;

	virtual void setLocation() = 0;
};

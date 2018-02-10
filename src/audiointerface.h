#pragma once
#include <memory>
#include <string>
#include "Util/vertex.h"

class IAudioResource;
class IAudioInstance;

struct SAudioInitParams
{
	Vec3 position;
	Vec3 velocity;
	float decayFactor = 1.0f;
	float decayDistance = 1.0f;
	float maxGain = 1.0f;
	float gain = 1.0f;
};

class IAudioDevice
{
public:
	virtual ~IAudioDevice() {}

	static void initialize();
	static void shutdown();
	static IAudioDevice& get();

	virtual bool checkStatus() = 0;
	virtual std::unique_ptr <IAudioResource> createAudioResource(std::string& filename) = 0;
	// play a resource once with the specified parameters
	virtual void playResourceOnce(const IAudioResource&, const SAudioInitParams&) = 0;
	// play a resource continuously, return a handle so that users can tweak the parameters dynamically
	virtual IAudioInstance* loopResource(const IAudioResource&, const SAudioInitParams&) = 0;
	virtual void deleteResource(IAudioInstance*) = 0;
	virtual void updateListener(Vec3 position, Vec3 orientation, Vec3 velocity) = 0;

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
	IAudioInstance() {}
	IAudioInstance(const IAudioInstance&) = delete;
	virtual ~IAudioInstance() {}

	virtual void setPosition(Vec3) = 0;
	virtual void setVelocity(Vec3) = 0;
	virtual void setPitchMultiplier(float) = 0;
};

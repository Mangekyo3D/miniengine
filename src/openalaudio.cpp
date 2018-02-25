#include "audiointerface.h"

#ifdef OPENAL_FOUND
#include <al.h>
#include <alc.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#define MAXONESHOTSOUNDS 30

#define READMEMBER(member) \
	file.read(reinterpret_cast<char *>(&member), sizeof(member))

class OpenALAudioResource : public IAudioResource
{
public:
	OpenALAudioResource(std::string& filename)
		: IAudioResource(filename)
		, m_buffer(0)
	{
		load();
	}

	~OpenALAudioResource()
	{
		unload();
	}

	ALuint getHandle() const { return m_buffer; }

	void load()
	{
		std::fstream file(m_filename, std::ios::in | std::ios::binary);

		// first unload the resource in case it has been loaded before
		unload();

		if (file)
		{
			// read file header according to wav file specification
			char magicchar[4];
			READMEMBER(magicchar);

			if (strncmp(magicchar, "RIFF", 4) !=0)
			{
				std::cout << "Audio file " << m_filename << " is not a RIFF/WAV file" << std::endl;
				return;
			}

			uint32_t fileSize;
			READMEMBER(fileSize);
			READMEMBER(magicchar);

			if (strncmp(magicchar, "WAVE", 4) !=0)
			{
				std::cout << "Audio file " << m_filename << " is not a WAV file" << std::endl;
				return;
			}

			// format chunk
			READMEMBER(magicchar);
			if (strncmp(magicchar, "fmt ", 4) !=0)
			{
				std::cout << "Audio file " << m_filename << " has incorrect format chunk" << std::endl;
				return;
			}

			// chunk size
			READMEMBER(fileSize);

			uint16_t formatWav;
			READMEMBER(formatWav);
			if (formatWav != 1)
			{
				std::cout << "Audio file " << m_filename << " has incorrect format" << std::endl;
				return;
			}

			uint16_t numChannels;
			READMEMBER(numChannels);
			uint32_t samplingRate;
			READMEMBER(samplingRate);
			uint32_t avgBytesPerSec;
			READMEMBER(avgBytesPerSec);
			uint16_t blockAlign;
			READMEMBER(blockAlign);
			uint16_t bitDepth;
			READMEMBER(bitDepth);

			// fix struct alignment issue on some wav writers.
			if (fileSize == 18)
			{
				uint16_t dummy;
				READMEMBER(dummy);
			}

			ALenum alFormat;

			if (numChannels == 1)
			{
				if (bitDepth == 8)
				{
					alFormat = AL_FORMAT_MONO8;
				}
				else if (bitDepth == 16)
				{
					alFormat = AL_FORMAT_MONO16;
				}
				else
				{
					std::cout << "Audio file " << m_filename << " unsupported bit depth " << bitDepth << std::endl;
					return;
				}
			}
			else if (numChannels == 2)
			{
				if (bitDepth == 8)
				{
					alFormat = AL_FORMAT_STEREO8;
				}
				else if (bitDepth == 16)
				{
					alFormat = AL_FORMAT_STEREO16;
				}
				else
				{
					std::cout << "Audio file " << m_filename << " unsupported bit depth " << bitDepth << std::endl;
					return;
				}
			}
			else
			{
				std::cout << "Audio file " << m_filename << " unsupported number of channels " << numChannels << std::endl;
				return;
			}

			READMEMBER(magicchar);
			if (strncmp(magicchar, "data", 4) !=0)
			{
				std::cout << "Audio file " << m_filename << " has incorrect data chunk" << std::endl;
				return;
			}

			uint32_t dataSize;
			READMEMBER(dataSize);

			if (dataSize > 0)
			{
				std::unique_ptr<char[]> soundData(new char[dataSize]);

				file.read(soundData.get(), dataSize);

				// data has been read, time to generate the openAL buffer
				alGenBuffers(1, &m_buffer);
				alBufferData(m_buffer, alFormat, soundData.get(), dataSize, samplingRate);
			}
		}
		else
		{
			std::cout << "Audio file " << m_filename << " was not found!" << std::endl;
		}
	}

	void unload()
	{
		if (m_buffer)
		{
			alDeleteBuffers(1, &m_buffer);
			m_buffer = 0;
		}
	}

private:
	ALuint m_buffer;
};

class OpenALAudioInstance : public IAudioInstance
{
public:
	OpenALAudioInstance(const IAudioResource& resource, const struct SAudioInitParams& params)
	{
		const OpenALAudioResource& alResource = static_cast<const OpenALAudioResource&> (resource);

		alGenSources(1, &m_source);

		alSourcei(m_source, AL_BUFFER, alResource.getHandle());
		alSourcefv(m_source, AL_POSITION, params.position.data());
		alSourcefv(m_source, AL_VELOCITY, params.velocity.data());
		alSourcef(m_source, AL_REFERENCE_DISTANCE, params.decayDistance);
		alSourcef(m_source, AL_ROLLOFF_FACTOR, params.decayFactor);
		alSourcef(m_source, AL_GAIN, params.gain);
		alSourcef(m_source, AL_MAX_GAIN, params.maxGain);
	}

	~OpenALAudioInstance()
	{
		alDeleteSources(1, &m_source);
	}

	virtual void setPosition(Vec3 pos)
	{
		alSourcefv(m_source, AL_POSITION, pos.data());
	}

	virtual void setVelocity(Vec3 vel)
	{
		alSourcefv(m_source, AL_VELOCITY, vel.data());
	}

	virtual void setPitchMultiplier(float mult)
	{
		alSourcef(m_source, AL_PITCH, mult);
	}

	void loop(bool bLoop)
	{
		alSourcei(m_source, AL_LOOPING, bLoop ? AL_TRUE : AL_FALSE);
	}

	void play()
	{
		alSourcePlay(m_source);
	}

	void stop()
	{
		alSourceStop(m_source);
	}

private:
	ALuint m_source;
};


class OpenALDevice : public IAudioDevice
{
	private:
		ALCcontext *alc;
		ALCdevice *aldev;
		// pool of one-shot sounds
		std::vector<ALuint> m_oneshotSounds;
		uint32_t m_currentOneShotSound;
		// looping sounds
		std::vector<std::unique_ptr <OpenALAudioInstance> > m_loopingSounds;

	public:
		OpenALDevice();
		~OpenALDevice();

		virtual bool checkStatus() override;
		virtual std::unique_ptr <IAudioResource> createAudioResource(std::string& filename);
		virtual void playResourceOnce(const IAudioResource&, const SAudioInitParams&);
		virtual IAudioInstance* loopResource(const IAudioResource&, const SAudioInitParams&);
		virtual void updateListener(Vec3 position, Vec3 orientation, Vec3 velocity);
		virtual void deleteResource(IAudioInstance*);
};

OpenALDevice::OpenALDevice()
	: m_oneshotSounds(MAXONESHOTSOUNDS)
{
	m_currentOneShotSound = 0;
	const char *device = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	aldev = alcOpenDevice(device);

	if(aldev == nullptr)
	{
		throw 0;
	}
	alc = alcCreateContext(aldev, nullptr);
	alcMakeContextCurrent(alc);
	alGetError();

	alGenSources(MAXONESHOTSOUNDS, &m_oneshotSounds[0]);
}

OpenALDevice::~OpenALDevice()
{
	// clear individual sound resources before clearing the
	alDeleteSources(MAXONESHOTSOUNDS, &m_oneshotSounds[0]);
	m_loopingSounds.clear();

	alc = alcGetCurrentContext();
	aldev = alcGetContextsDevice(alc);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(alc);
	alcCloseDevice(aldev);

	// default values for the sound module
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alSpeedOfSound(3.0);
	alDopplerFactor(4.0);
}

bool OpenALDevice::checkStatus()
{
#define HANDLE_ENUM_ERR(e) \
	case e: \
	std::cout << "OpenAL error: " #e << std::endl; \
	break;

	ALenum errorCode = alGetError();

	switch (errorCode)
	{
		case AL_NO_ERROR:
			return true;

		HANDLE_ENUM_ERR(AL_INVALID_NAME)
		HANDLE_ENUM_ERR(AL_INVALID_ENUM)
		HANDLE_ENUM_ERR(AL_INVALID_VALUE)
		HANDLE_ENUM_ERR(AL_INVALID_OPERATION)
		HANDLE_ENUM_ERR(AL_OUT_OF_MEMORY)

		default:
			break;
	}
	return false;

#undef HANDLE_ENUM_ERR
}


std::unique_ptr <IAudioResource>OpenALDevice::createAudioResource(std::string& filename)
{
	return std::make_unique <OpenALAudioResource> (filename);
}

void OpenALDevice::playResourceOnce(const IAudioResource &resource, const SAudioInitParams &params)
{
	ALuint source = m_oneshotSounds[m_currentOneShotSound++];
	m_currentOneShotSound %= MAXONESHOTSOUNDS;

	const OpenALAudioResource& alResource = static_cast<const OpenALAudioResource&> (resource);

	// stop the source, in case it is still playing
	alSourceStop(source);
	alSourcefv(source, AL_POSITION, params.position.data());
	alSourcefv(source, AL_VELOCITY, params.velocity.data());
	alSourcei(source, AL_BUFFER, alResource.getHandle());
	alSourcef(source, AL_REFERENCE_DISTANCE, params.decayDistance);
	alSourcef(source, AL_ROLLOFF_FACTOR, params.decayFactor);
	alSourcef(source, AL_GAIN, params.gain);
	alSourcef(source, AL_MAX_GAIN, params.maxGain);
	alSourcePlay(source);
}

IAudioInstance* OpenALDevice::loopResource(const IAudioResource &resource, const SAudioInitParams &params)
{
	auto newSound = std::make_unique<OpenALAudioInstance> (resource, params);
	newSound->loop(true);
	newSound->play();
	IAudioInstance* ret = static_cast<IAudioInstance*> (newSound.get());
	m_loopingSounds.push_back(std::move(newSound));
	return ret;
}

void OpenALDevice::updateListener(Vec3 position, Vec3 orientation, Vec3 velocity)
{
	alListenerfv(AL_POSITION, position.data());
	alListenerfv(AL_ORIENTATION, orientation.data());
	alListenerfv(AL_VELOCITY, velocity.data());
}

void OpenALDevice::deleteResource(IAudioInstance* res)
{
	for (auto iter = m_loopingSounds.begin(); iter < m_loopingSounds.end(); ++iter)
	{
		if (iter->get() == res)
		{
			m_loopingSounds.erase(iter);
			break;
		}
	}
}

#endif

class NullAudioResource : public IAudioResource
{
public:
	NullAudioResource(std::string& filename)
		: IAudioResource(filename)
	{}
	~NullAudioResource() {}
};

class NullAudioInstance : public IAudioInstance
{
public:
	NullAudioInstance() {}
	~NullAudioInstance() {}

	virtual void setPosition(Vec3) {}
	virtual void setVelocity(Vec3) {}
	virtual void setPitchMultiplier(float) {}
};


class CNullAudioDevice : public IAudioDevice
{
public:
	virtual bool checkStatus() override { return true; }
	virtual std::unique_ptr <IAudioResource> createAudioResource(std::string& filename) override { return std::make_unique<NullAudioResource> (filename); }
	virtual void playResourceOnce(const IAudioResource&, const SAudioInitParams&) override {}
	virtual IAudioInstance* loopResource(const IAudioResource&, const SAudioInitParams&) override { return &m_dummyResource;}
	virtual void updateListener(Vec3 position, Vec3 orientation, Vec3 velocity) override {}
	virtual void deleteResource(IAudioInstance*) override {}

private:
	NullAudioInstance m_dummyResource;
};

IAudioDevice& IAudioDevice::get()
{
	return *s_audioDevice;
}

std::unique_ptr <IAudioDevice> IAudioDevice::s_audioDevice;

void IAudioDevice::initialize()
{
#ifdef OPENAL_FOUND
	s_audioDevice.reset(new OpenALDevice);
#else
	s_audioDevice.reset(new CNullAudioDevice);
#endif
}

void IAudioDevice::shutdown()
{
	s_audioDevice.reset();
}

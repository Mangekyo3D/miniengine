#include "audiointerface.h"

#ifdef OPENAL_FOUND
#include <AL/alut.h>
#include <AL/alc.h>
#include <iostream>
#include <fstream>

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


	void load()
	{
		std::ifstream file(m_filename);

		// first unload the resource in case it has been loaded before
		unload();

		if (file)
		{
			std::cout << "Audio file " << m_filename << " found!" << std::endl;

			// read file header according to wav file specification

			// data has been read, time to generate the buffer
			alGenBuffers(1, &m_buffer);
			alBufferData(m_buffer, format, data, size, frequency);
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

class OpenALDevice : public IAudioDevice
{
	private:
		ALCcontext *alc;
		ALCdevice *aldev;

	public:
		OpenALDevice();
		~OpenALDevice();

		virtual bool checkStatus() override;
		virtual std::unique_ptr <IAudioResource> createAudioResource(std::string& filename);
};

OpenALDevice::OpenALDevice()
{
	const char *device = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	aldev = alcOpenDevice(device);

	if(aldev == nullptr)
	{
		throw 0;
	}
	alc = alcCreateContext(aldev, nullptr);
	alcMakeContextCurrent(alc);
	alGetError();
}

OpenALDevice::~OpenALDevice()
{
	alc = alcGetCurrentContext();
	aldev = alcGetContextsDevice(alc);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(alc);
	alcCloseDevice(aldev);
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

#endif

class CNullAudioDevice : public IAudioDevice
{
		virtual bool checkStatus() override { return true; }
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

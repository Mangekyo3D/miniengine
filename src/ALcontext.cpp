#include "ALcontext.h"

#ifdef OPENAL_FOUND
#include <AL/alut.h>
#include <AL/alc.h>
#include <iostream>

class OpenALDevice : public IAudioDevice
{
	private:
		ALCcontext *alc;
		ALCdevice *aldev;

	public:
		OpenALDevice();
		~OpenALDevice();

		virtual bool checkStatus() override;
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
	//alutInitWithoutContext(nullptr, nullptr);
}

OpenALDevice::~OpenALDevice()
{
	alc = alcGetCurrentContext();
	aldev = alcGetContextsDevice(alc);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(alc);
	alcCloseDevice(aldev);
	//alutExit();
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

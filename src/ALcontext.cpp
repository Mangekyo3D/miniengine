#include "ALcontext.h"

#ifdef OPENAL_FOUND
OpenALContext::OpenALContext()
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
	alutInitWithoutContext(nullptr, nullptr);
}

OpenALContext::~OpenALContext()
{
	alc = alcGetCurrentContext();
	aldev = alcGetContextsDevice(alc);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(alc);
	alcCloseDevice(aldev);
	alutExit();
}
#endif

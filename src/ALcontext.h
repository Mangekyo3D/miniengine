#pragma once

class CAudioDevice
{
	public:
		virtual ~CAudioDevice() {}
		virtual bool checkStatus() = 0;
};

class CNullAudioDevice : public CAudioDevice
{
		virtual bool checkStatus() override { return true; }
};

#ifdef OPENAL_FOUND
#include <AL/alut.h>
#include <AL/alc.h>
class OpenALContext : public CAudioDevice
{
	private:
		ALCcontext *alc;
		ALCdevice *aldev;

	public:
		OpenALContext();
		~OpenALContext();

		virtual bool checkStatus() override { 
			return alGetError() != AL_NO_ERROR;
		}
};
#endif

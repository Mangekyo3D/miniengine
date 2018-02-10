#include "audiointerface.h"

#ifdef OPENAL_FOUND
#include <al.h>
#include <alc.h>
#include <iostream>
#include <fstream>

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


	void load()
	{
		std::ifstream file(m_filename);

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
		virtual std::unique_ptr <IAudioResource> createAudioResource(std::string& filename) { return nullptr; }
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

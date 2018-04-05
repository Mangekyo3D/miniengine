#pragma once
#include <map>
#include <memory>
#include <array>

class SMDModel;
class IPipeline;
class ITexture;
class IAudioResource;
class IDevice;

enum EPipelines {
	eDiffuse = 0,
	eDiffuseTextured,

	// post processing pipelines
	eToneMapping,
	eMaxPipelines
};

class ResourceManager
{
	public:
		ResourceManager(IDevice* device);
		~ResourceManager();
		ResourceManager(const ResourceManager&) = delete;

		SMDModel* loadModel(std::string modelName);
		IPipeline* loadPipeline(EPipelines pipeline);
		ITexture* loadTexture(std::string textureName);
		IAudioResource* loadAudio(std::string audioName);

	private:
		void loadDefaultPipelines();

		std::map <std::string, std::unique_ptr<SMDModel> > m_models;
		std::array <std::unique_ptr<IPipeline>, eMaxPipelines > m_pipelines;
		std::map <std::string, std::unique_ptr<ITexture> > m_textures;
		std::map <std::string, std::unique_ptr<IAudioResource> > m_audio;

		IDevice* m_device;
};


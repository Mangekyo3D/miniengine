#pragma once
#include <map>
#include <memory>

class SMDModel;
class PipelineObject;
class ITexture;
class IAudioResource;
class IDevice;

class ResourceManager
{
	public:
		ResourceManager(IDevice* device);
		~ResourceManager();
		ResourceManager(const ResourceManager&) = delete;

		SMDModel* loadModel(std::string modelName);
		PipelineObject* loadPipeline(std::string pipelineName);
		ITexture* loadTexture(std::string textureName);
		IAudioResource* loadAudio(std::string audioName);

	private:
		void loadDefaultPipelines();

		std::map <std::string, std::unique_ptr<SMDModel> > m_models;
		std::map <std::string, std::unique_ptr<PipelineObject> > m_pipelines;
		std::map <std::string, std::unique_ptr<ITexture> > m_textures;
		std::map <std::string, std::unique_ptr<IAudioResource> > m_audio;

		IDevice* m_device;
};


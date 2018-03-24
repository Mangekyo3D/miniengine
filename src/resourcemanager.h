#pragma once
#include <map>
#include <memory>

class SMDModel;
class PipelineObject;
class CTexture;
class IAudioResource;

class ResourceManager
{
	public:
		~ResourceManager();
		static ResourceManager& get() { return s_manager; }
		ResourceManager(const ResourceManager&) = delete;

		SMDModel* loadModel(std::string modelName);
		PipelineObject* loadPipeline(std::string pipelineName);
		CTexture* loadTexture(std::string textureName);
		IAudioResource* loadAudio(std::string audioName);

		void loadDefaultPipelines();
		void cleanup();

	private:
		static ResourceManager s_manager;
		ResourceManager();

		std::map <std::string, std::unique_ptr<SMDModel> > m_models;
		std::map <std::string, std::unique_ptr<PipelineObject> > m_pipelines;
		std::map <std::string, std::unique_ptr<CTexture> > m_textures;
		std::map <std::string, std::unique_ptr<IAudioResource> > m_audio;
};


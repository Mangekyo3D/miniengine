#include "resourcemanager.h"
#include "SMDmodel.h"
#include "batch.h"
#include "render/itexture.h"
#include "OS/OSFactory.h"
#include "audiointerface.h"
#include "render/idevice.h"
#include "bitmap.h"

ResourceManager::ResourceManager(IDevice* device)
	: m_device(device)
{
	loadDefaultPipelines();
}

ResourceManager::~ResourceManager()
{
}

SMDModel* ResourceManager::loadModel(std::string modelName)
{
	auto iter = m_models.find(modelName);
	if (iter != m_models.end())
	{
		return iter->second.get();
	}
	else
	{
		auto& utils =  OSUtils::get();
		std::string finalFilename = utils.getModelPath() + modelName;

		auto model = std::make_unique <SMDModel> ();

		if (model->openFromFile(this, finalFilename.c_str()))
		{
			SMDModel* result = model.get();
			m_models[modelName] = std::move(model);
			return result;
		}
		else return nullptr;
	}
}

COpenGLPipeline* ResourceManager::loadPipeline(EPipelines pipelineName)
{
	return m_pipelines[pipelineName].get();
}

ITexture* ResourceManager::loadTexture(std::string textureName)
{
	auto iter = m_textures.find(textureName);
	if (iter != m_textures.end())
	{
		return iter->second.get();
	}
	else
	{
		auto& utils =  OSUtils::get();
		std::string finalFileName = utils.getTexturePath() + textureName;

		BmpReader reader;
		if (reader.openFromFile(finalFileName.data(), true))
		{
			auto texture = m_device->createTexture(ITexture::EFormat::eSRGB8, reader.getWidth(), reader.getHeight(), true);
			ITexture* result = texture.get();
			TextureStreamRequest req(result, finalFileName.data());
			m_device->addTextureStreamRequest(req);
			m_textures[textureName] = std::move(texture);
			return result;
		}
		else
		{
			return nullptr;
		}
	}
}

IAudioResource *ResourceManager::loadAudio(std::string audioName)
{
	auto iter = m_audio.find(audioName);
	if (iter != m_audio.end())
	{
		return iter->second.get();
	}
	else
	{
		auto& utils =  OSUtils::get();
		std::string finalFileName = utils.getAudioPath() + audioName;

		auto audio = IAudioDevice::get().createAudioResource(finalFileName);
		IAudioResource* result = audio.get();
		m_audio[audioName] = std::move(audio);
		return result;
	}
}

void ResourceManager::loadDefaultPipelines()
{
	std::string pipelineName = "generic";
	auto pipeline = std::make_unique <COpenGLPipeline> (pipelineName, std::make_unique <IndexedInstancedDescriptorV> ());
	m_pipelines[eDiffuse] = std::move(pipeline);

	pipelineName = "genericTextured";
	pipeline = std::make_unique <COpenGLPipeline> (pipelineName, std::make_unique <IndexedInstancedDescriptorVT> ());
	m_pipelines[eDiffuseTextured] = std::move(pipeline);

	pipelineName = "toneMapping";
	pipeline = std::make_unique <COpenGLPipeline> (pipelineName, std::make_unique <ArrayDescriptorV> ());
	m_pipelines[eToneMapping] = std::move(pipeline);
}

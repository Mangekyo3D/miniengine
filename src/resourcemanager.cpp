#include "resourcemanager.h"
#include "SMDmodel.h"
#include "batch.h"
#include "texture.h"
#include "OS/OSFactory.h"
#include "audiointerface.h"

ResourceManager ResourceManager::s_manager;

ResourceManager::ResourceManager()
{
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

		if (model->openFromFile(finalFilename.c_str()))
		{
			SMDModel* result = model.get();
			m_models[modelName] = std::move(model);
			return result;
		}
		else return nullptr;
	}
}

PipelineObject* ResourceManager::loadMaterial(std::string materialName)
{
	auto iter = m_materials.find(materialName);
	if (iter != m_materials.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}

CTexture* ResourceManager::loadTexture(std::string textureName)
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

		auto texture = std::make_unique <CTexture> (finalFileName, true);
		CTexture* result = texture.get();
		m_textures[textureName] = std::move(texture);
		return result;
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

void ResourceManager::initialize()
{
	std::string materialName = "generic";
	auto material = std::make_unique <PipelineObject> (materialName, std::make_unique <IndexedInstancedDescriptorV> ());
	m_materials[materialName] = std::move(material);

	materialName = "genericTextured";
	material = std::make_unique <PipelineObject> (materialName, std::make_unique <IndexedInstancedDescriptorVT> ());
	m_materials[materialName] = std::move(material);
}

void ResourceManager::cleanup()
{
	m_materials.clear();
	m_models.clear();
	m_textures.clear();
}

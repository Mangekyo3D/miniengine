#include "resourcemanager.h"
#include "SMDmodel.h"
#include "batch.h"
#include "texture.h"
#include "OS/OSFactory.h"

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

Material* ResourceManager::loadMaterial(std::string materialName)
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

void ResourceManager::initialize()
{
	// initialize standard material library
	auto& utils =  OSUtils::get();

	std::string materialName = "generic";
	std::string shaderFileName = utils.getShaderPath() + materialName;
	auto material = std::make_unique <Material> (shaderFileName, std::make_unique <GenericMaterialDescriptor> ());
	m_materials[materialName] = std::move(material);

	materialName = "genericTextured";
	shaderFileName = utils.getShaderPath() + materialName;
	material = std::make_unique <Material> (shaderFileName, std::make_unique <TexturedMaterialDescriptor> ());
	m_materials[materialName] = std::move(material);
}

void ResourceManager::cleanup()
{
	m_materials.clear();
	m_models.clear();
	m_textures.clear();
}

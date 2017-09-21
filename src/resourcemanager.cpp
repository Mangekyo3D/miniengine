#include "resourcemanager.h"
#include "SMDmodel.h"
#include "batch.h"
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
	auto& utils =  OSUtils::get();
	std::string finalFilename = utils.getModelPath() + modelName;

	auto iter = m_models.find(finalFilename);
	if (iter != m_models.end())
	{
		return iter->second.get();
	}
	else
	{
		auto model = std::make_unique <SMDModel> ();

		if (model->openFromFile(finalFilename.c_str()))
		{
			SMDModel* result = model.get();
			m_models[finalFilename] = std::move(model);
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
		auto material = std::make_unique <Material> (materialName);
		Material* result = material.get();
		m_materials[materialName] = std::move(material);
		return result;
	}
}

void ResourceManager::cleanup()
{
	m_materials.clear();
	m_models.clear();
}

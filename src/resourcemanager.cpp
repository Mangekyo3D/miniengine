#include "resourcemanager.h"
#include "SMDmodel.h"
#include "OS/OSFactory.h"

ResourceManager ResourceManager::s_manager;

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
			m_models[finalFilename] = std::move(model);
			return model.get();
		}
		else return nullptr;
	}
}

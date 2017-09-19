#pragma once
#include <map>
#include <memory>

class SMDModel;

class ResourceManager
{
	public:
		static ResourceManager& get() { return s_manager; }
		SMDModel* loadModel(std::string modelName);

	private:
		static ResourceManager s_manager;
		ResourceManager() {}
		std::map <std::string, std::unique_ptr<SMDModel> > m_models;
};


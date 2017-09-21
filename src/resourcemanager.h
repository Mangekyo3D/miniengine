#pragma once
#include <map>
#include <memory>

class SMDModel;
class Material;

class ResourceManager
{
	public:
		~ResourceManager();
		static ResourceManager& get() { return s_manager; }
		ResourceManager(const ResourceManager&) = delete;

		SMDModel* loadModel(std::string modelName);
		Material* loadMaterial(std::string materialName);

		void cleanup();

	private:
		static ResourceManager s_manager;
		ResourceManager();

		std::map <std::string, std::unique_ptr<SMDModel> > m_models;
		std::map <std::string, std::unique_ptr<Material> > m_materials;
};


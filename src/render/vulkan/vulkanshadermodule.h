#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class CVulkanShaderModule
{
public:
	CVulkanShaderModule(const std::string& filename);
	~CVulkanShaderModule();
	void load();

	VkShaderModule getModule() { return m_module; }

private:
	// this only stores filename inside /shaders directory, not full path
	std::string      m_filename;
	VkShaderModule m_module;

	// size of binary module in bytes and actual data
	std::vector<unsigned char> m_data;
};

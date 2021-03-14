#include "vulkanshadermodule.h"
#include "vulkandevice.h"
#include "../../OS/OSFactory.h"
#include <fstream>
#include <iostream>

CVulkanShaderModule::CVulkanShaderModule(VkShaderModule module)
	: m_module(module)
{
}

CVulkanShaderModule::~CVulkanShaderModule()
{
	if (m_module == VK_NULL_HANDLE)
		return;
	auto& device = CVulkanDevice::get();
	device.vkDestroyShaderModule(device, m_module, nullptr);
}

std::optional<CVulkanShaderModule> CVulkanShaderModule::create(const std::string& path)
{
	const std::string filename = OSUtils::get().getShaderPath() + path;
	std::ifstream file(filename.data(), std::ios_base::in | std::ios_base::binary);

	std::vector<unsigned char> binary_data;
	if (!file)
	{
		std::cout << "File " << filename << " can't be opened for reading" << std::endl;
		return std::nullopt;
	}

	// classic trick to get length of file, seek to end, get
	file.seekg(0, std::ios_base::end);
	binary_data.reserve(static_cast<size_t>(file.tellg()));
	file.seekg(0, std::ios_base::beg);

	binary_data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	if (binary_data.empty())
		return std::nullopt;

	auto& device = CVulkanDevice::get();
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		binary_data.size(),
		reinterpret_cast<uint32_t*> (binary_data.data())
	};

	VkShaderModule module;
	if (device.vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module) != VK_SUCCESS)
	{
        std::cout << "Error during shader module creation" << std::endl;
	}

	return std::make_optional(CVulkanShaderModule(module));
}

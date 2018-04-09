#include "vulkanshadermodule.h"
#include "vulkandevice.h"
#include "../../OS/OSFactory.h"
#include <fstream>
#include <iostream>

CVulkanShaderModule::CVulkanShaderModule(const std::string& filename)
{
	m_filename = OSUtils::get().getShaderPath() + filename;
	load();
}

CVulkanShaderModule::~CVulkanShaderModule()
{
	if (m_module)
	{
		auto& device = CVulkanDevice::get();
		device.vkDestroyShaderModule(device, m_module, nullptr);
	}
}

void CVulkanShaderModule::load()
{
	if (!m_data.empty())
	{
		return;
	}

	// first construct a filename from the current directory, appending /shaders to it
	std::ifstream file(m_filename.data(), std::ios_base::in | std::ios_base::binary);

	if (file)
	{
		// classic trick to get length of file, seek to end, get
		file.seekg(0, std::ios_base::end);
		m_data.reserve(file.tellg());
		file.seekg(0, std::ios_base::beg);

		m_data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		file.close();
	}
	else
	{
		std::cout << "File " << m_filename << " can't be opened for reading" << std::endl;;
	}

	if (!m_data.empty())
	{
		auto& device = CVulkanDevice::get();

		VkShaderModuleCreateInfo shaderModuleCreateInfo = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			nullptr,
			0,
			m_data.size(),
			reinterpret_cast<uint32_t*> (m_data.data())
		};

		if (device.vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &m_module) != VK_SUCCESS)
		{
			std::cout << "Error during shader module creation" << std::endl;;
		}
	}
}

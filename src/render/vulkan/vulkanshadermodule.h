#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <optional>

class CVulkanShaderModule
{
public:
	~CVulkanShaderModule();
	CVulkanShaderModule(const CVulkanShaderModule&) = delete;
	CVulkanShaderModule(CVulkanShaderModule&& anOther) noexcept {
		m_module = anOther.m_module; anOther.m_module = VK_NULL_HANDLE;
	}

	static std::optional<CVulkanShaderModule> create(const std::string& path);
	VkShaderModule getModule() { return m_module; }

private:
	CVulkanShaderModule(VkShaderModule module);

	VkShaderModule m_module;
};

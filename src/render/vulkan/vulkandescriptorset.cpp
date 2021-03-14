#include "vulkandescriptorset.h"
#include "vulkandevice.h"
#include "../ipipeline.h"
#include <iostream>

CVulkanDescriptorSet::CVulkanDescriptorSet(SDescriptorSet& params, VkSampler* samplers)
	: m_setLayout(VK_NULL_HANDLE)
{
	auto& device = CVulkanDevice::get();

	std::vector <VkDescriptorSetLayoutBinding> layoutBindings;

	for (auto& descriptor : params.descriptors)
	{
		VkDescriptorType type = descriptorToVulkanType(descriptor.type);
		layoutBindings.push_back(
					VkDescriptorSetLayoutBinding {
						static_cast <uint32_t> (layoutBindings.size()),
						type,
						1,
						stageFlagsToVulkanFlags(descriptor.shaderStages),
						(descriptor.type == EDescriptorType::eTextureSampler && samplers) ? &samplers[descriptor.sampler] : nullptr
					});

		m_descriptorPoolConfig.push_back({type, 1});
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast <uint32_t> (layoutBindings.size()),
		layoutBindings.data()
	};

	if (device.vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &m_setLayout) != VK_SUCCESS)
	{
		std::cout << "Error during descriptor set layout creation" << std::endl;
	}
}

CVulkanDescriptorSet::~CVulkanDescriptorSet()
{
	auto& device = CVulkanDevice::get();

	if (m_setLayout)
	{
		device.vkDestroyDescriptorSetLayout(device, m_setLayout, nullptr);
	}
}

CDescriptorPool CVulkanDescriptorSet::createDescriptorPool(uint32_t numDescriptors)
{
	return CDescriptorPool(m_setLayout, m_descriptorPoolConfig.data(), static_cast <uint32_t> (m_descriptorPoolConfig.size()), numDescriptors);
}

VkDescriptorType CVulkanDescriptorSet::descriptorToVulkanType(EDescriptorType desc)
{
	switch (desc)
	{
		case EDescriptorType::eUniformBlock:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case EDescriptorType::eTextureSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}


CDescriptorPool::CDescriptorPool(VkDescriptorSetLayout setLayout, VkDescriptorPoolSize* perDesrInfo, uint32_t numDecr, uint32_t numSets)
	: m_setLayout(setLayout)
{
	auto& device = CVulkanDevice::get();

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		numSets,
		numDecr,
		perDesrInfo
	};

	if (device.vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_pool) != VK_SUCCESS)
	{
		std::cout << "Error during descriptor pool creation" << std::endl;
	}
}

CDescriptorPool::~CDescriptorPool()
{
	if (m_pool == VK_NULL_HANDLE)
		return;
	auto& device = CVulkanDevice::get();
	device.vkDestroyDescriptorPool(device, m_pool, nullptr);
}

VkDescriptorSet CDescriptorPool::allocate()
{
	VkDescriptorSetAllocateInfo allocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,
		m_pool,
		1,
		&m_setLayout
	};

	auto& device = CVulkanDevice::get();
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkResult result = device.vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet);
	if (result != VK_SUCCESS)
	{
		std::cout << "Error during descriptor set allocation" << std::endl;
	}

	return descriptorSet;
}

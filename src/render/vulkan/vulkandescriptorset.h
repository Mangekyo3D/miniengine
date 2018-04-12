#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "../ipipeline.h"

struct SDescriptorSet;

class CDescriptorPool
{
	public:
		CDescriptorPool(VkDescriptorSetLayout setLayout, VkDescriptorPoolSize* perDesrInfo, uint32_t numDecr, uint32_t numSets);
		CDescriptorPool(const CDescriptorPool&) = delete;
		CDescriptorPool& operator = (const CDescriptorPool&) = delete;
		~CDescriptorPool();
		VkDescriptorSet allocate();

	private:
		VkDescriptorSetLayout m_setLayout;
		VkDescriptorPool m_pool;
		std::vector <VkDescriptorSet> m_sets;
};

class CVulkanDescriptorSet
{
	public:
		CVulkanDescriptorSet(SDescriptorSet& params, VkSampler* samplers = nullptr);
		CVulkanDescriptorSet(const CVulkanDescriptorSet&) = delete;
		operator VkDescriptorSetLayout () { return m_setLayout; }
		~CVulkanDescriptorSet();

		std::unique_ptr <CDescriptorPool> createDescriptorPool(uint32_t numSets);

	private:
		VkDescriptorType descriptorToVulkanType(EDescriptorType desc);

		VkDescriptorSetLayout m_setLayout;
		std::vector <VkDescriptorPoolSize> m_descriptorPoolConfig;
};

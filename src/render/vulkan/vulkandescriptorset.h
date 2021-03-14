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
		CDescriptorPool(CDescriptorPool&& anOther) noexcept {
			*this = std::move(anOther);
		}
		CDescriptorPool& operator = (const CDescriptorPool&) = delete;
		CDescriptorPool& operator = (CDescriptorPool&& anOther) noexcept {
			m_setLayout = anOther.m_setLayout;
			m_pool = anOther.m_pool;
			m_sets = std::move(anOther.m_sets);
			anOther.m_setLayout = VK_NULL_HANDLE;
			anOther.m_pool = VK_NULL_HANDLE;
			return *this;
		}
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
		CVulkanDescriptorSet& operator = (const CVulkanDescriptorSet&) = delete;
		operator VkDescriptorSetLayout () { return m_setLayout; }
		~CVulkanDescriptorSet();

		CDescriptorPool createDescriptorPool(uint32_t numSets);

	private:
		VkDescriptorType descriptorToVulkanType(EDescriptorType desc);

		VkDescriptorSetLayout m_setLayout;
		std::vector <VkDescriptorPoolSize> m_descriptorPoolConfig;
};

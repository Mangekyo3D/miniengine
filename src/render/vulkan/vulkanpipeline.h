#pragma once
#include "../ipipeline.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class IRenderPass;

struct SDescriptorPool
{
	SDescriptorPool(VkDescriptorPoolSize* perDesrInfo, uint32_t numDecr, uint32_t numSets);
	SDescriptorPool(const SDescriptorPool&) = delete;
	SDescriptorPool& operator = (const SDescriptorPool&) = delete;
	~SDescriptorPool();

	VkDescriptorPool m_pool;
};

class CVulkanPipeline : public IPipeline
{
	public:
		CVulkanPipeline(SPipelineParams& params);
		~CVulkanPipeline();
		std::unique_ptr <SDescriptorPool> createPerFrameDescriptorPool(size_t numDescriptors);
		std::unique_ptr <SDescriptorPool> createGlobalPool();

		operator VkPipeline () {return m_pipeline; }

	private:
		VkFormat attributeParamToVertFormat(SVertexAttribParams&);
		VkShaderStageFlags stageFlagsToVulkanFlags(uint32_t stages);
		VkDescriptorType descriptorToVulkanType(EDescriptorType desc);
		VkDescriptorSetLayout createSetLayout(SDescriptorSet*);

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		VkDescriptorSetLayout m_globaLayout;
		VkDescriptorSetLayout m_perDrawLayout;

		std::vector <VkSampler> m_samplers;
};

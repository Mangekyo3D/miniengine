#pragma once
#include "../ipipeline.h"
#include <vulkan/vulkan.h>
#include <vector>

class IRenderPass;

struct SDescriptorPool
{
	SDescriptorPool(VkDescriptorPoolSize* perDesrInfo, uint32_t numDecr);
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
		virtual void setRequiredPerFrameDescriptors(size_t numDescriptors);
		operator VkPipeline () {return m_pipeline; }

	private:
		VkFormat attributeParamToVertFormat(SVertexAttribParams&);
		VkShaderStageFlags stageFlagsToVulkanFlags(uint32_t stages);
		VkDescriptorType descriptorToVulkanType(EDescriptorType desc);
		VkDescriptorSetLayout createSet(SDescriptorSet*);

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		VkDescriptorSetLayout m_globaLayout;
		VkDescriptorSetLayout m_perDrawLayout;

		std::vector <VkSampler> m_samplers;
};

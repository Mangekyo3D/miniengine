#pragma once
#include "../ipipeline.h"
#include <vulkan/vulkan.h>
#include <vector>

class IRenderPass;

class CVulkanPipeline : public IPipeline
{
	public:
		CVulkanPipeline(SPipelineParams& params);
		~CVulkanPipeline();
		operator VkPipeline () {return m_pipeline; }

	private:
		VkFormat attributeParamToVertFormat(SVertexAttribParams&);
		VkShaderStageFlags stageFlagsToVulkanFlags(uint32_t stages);
		VkDescriptorType descriptorToVulkanType(EDescriptorType desc);

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
		VkDescriptorSetLayout m_descriptorSetLayout;
		std::vector <VkSampler> m_samplers;
};

#pragma once
#include "../ipipeline.h"
#include <vulkan/vulkan.h>

class IRenderPass;

class CVulkanPipeline : public IPipeline
{
	public:
		CVulkanPipeline(IRenderPass& renderpass, SPipelineParams& params, SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding, const char* shaderName);
		~CVulkanPipeline();
		operator VkPipeline () {return m_pipeline; }

	private:
		VkFormat attributeParamToVertFormat(SVertexAttribParams&);

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
		VkDescriptorSetLayout m_descriptorSetLayout;
};

#pragma once
#include "../ipipeline.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class CVulkanDescriptorSet;
class IRenderPass;

class CVulkanPipeline : public IPipeline
{
	public:
		CVulkanPipeline(SPipelineParams& params);
		~CVulkanPipeline();

		operator VkPipeline () {return m_pipeline; }
		CVulkanDescriptorSet* getGlobalSet() { return m_globaLayout.get(); }
		CVulkanDescriptorSet* getPerDrawSet() { return m_perDrawLayout.get(); }

	private:
		VkFormat attributeParamToVertFormat(SVertexAttribParams&);

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		std::unique_ptr <CVulkanDescriptorSet> m_globaLayout;
		std::unique_ptr <CVulkanDescriptorSet> m_perDrawLayout;

		std::vector <VkSampler> m_samplers;
};

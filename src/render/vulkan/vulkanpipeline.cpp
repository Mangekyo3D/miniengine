#include "vulkanpipeline.h"
#include "vulkanshadermodule.h"
#include "vulkandevice.h"
#include "vulkanrenderpass.h"
#include <string>
#include <array>
#include <vector>
#include <iostream>

CVulkanPipeline::CVulkanPipeline(IRenderPass& renderpass, SPipelineParams& params, SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding, const char* shaderName)
{
	auto& device = CVulkanDevice::get();
	std::string filename = shaderName;
	CVulkanShaderModule fragmentShader(filename + ".frag.spv");
	CVulkanShaderModule vertexShader(filename + ".vert.spv");
	CVulkanRenderPass& rpass = static_cast <CVulkanRenderPass&> (renderpass);

	VkPipelineShaderStageCreateInfo pipelineShaderStages[] =
	{
		VkPipelineShaderStageCreateInfo {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			vertexShader.getModule(),
			"main",
			nullptr
		},
		VkPipelineShaderStageCreateInfo {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			fragmentShader.getModule(),
			"main",
			nullptr
		}
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		((params.m_flags & ePrimitiveRestart) != 0)
	};

	VkPipelineViewportStateCreateInfo viewportInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1,
		nullptr,
		1,
		nullptr
	};

	VkVertexInputBindingDescription vertexBindings[2];
	std::vector<VkVertexInputAttributeDescription> vertexAttributes;

	uint32_t numVertBindings = 0;

	if (perVertBinding)
	{
		vertexBindings[numVertBindings++] =
			VkVertexInputBindingDescription {
				0,
				static_cast<uint32_t> (perVertBinding->m_dataSize),
				VK_VERTEX_INPUT_RATE_VERTEX
			};

		for (auto& attribs : perVertBinding->m_attributeParams)
		{
			vertexAttributes.emplace_back(
				VkVertexInputAttributeDescription {
					static_cast<uint32_t> (vertexAttributes.size()),
					0,
					attributeParamToVertFormat(attribs),
					attribs.offset
				});
		}
	}

	if (perInstanceBinding)
	{
		vertexBindings[numVertBindings++] =
			VkVertexInputBindingDescription {
				1,
				static_cast<uint32_t> (perInstanceBinding->m_dataSize),
				VK_VERTEX_INPUT_RATE_INSTANCE
			};

		for (auto& attribs : perInstanceBinding->m_attributeParams)
		{
			vertexAttributes.push_back(
				VkVertexInputAttributeDescription {
					static_cast<uint32_t> (vertexAttributes.size()),
					1,
					attributeParamToVertFormat(attribs),
					attribs.offset
				});
		}
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		numVertBindings,
		vertexBindings,
		static_cast <uint32_t> (vertexAttributes.size()),
		vertexAttributes.data()
	};

	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		0,
		0,
		VK_POLYGON_MODE_FILL,
		static_cast <VkCullModeFlags> ((params.m_flags & eCullBackFace) ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE),
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		0,
		0,
		0,
		0,
		1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisampleInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		0,
		1.0f,
		nullptr,
		0,
		0
	};

	VkStencilOpState stencilStateCommon = {
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_KEEP,
		VK_COMPARE_OP_NEVER,
		0,
		0,
		0
	};

	VkCompareOp depthCompOp = VK_COMPARE_OP_ALWAYS;
	bool bDepthCompare = VK_FALSE;
	bool bDepthWrite = VK_TRUE;

	if (params.m_flags & eDepthCompareGreater)
	{
		depthCompOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
		bDepthCompare = VK_TRUE;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		nullptr,
		0,
		bDepthCompare,
		bDepthWrite,
		depthCompOp,
		VK_FALSE,
		VK_FALSE,
		stencilStateCommon,
		stencilStateCommon,
		0.0f,
		1.0f
	};

	VkPipelineColorBlendAttachmentState blendState = {
		VK_FALSE,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo blendInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		0,
		VK_LOGIC_OP_NO_OP,
		1,
		&blendState,
		{1.0f, 1.0f, 1.0f, 1.0f}
	};

	std::array <VkDynamicState, 2> dynamicStates{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		static_cast <uint32_t> (dynamicStates.size()),
		dynamicStates.data()
	};

	std::array <VkDescriptorSetLayoutBinding, 1> layoutBindings
	{
		VkDescriptorSetLayoutBinding {
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		}
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		static_cast <uint32_t> (layoutBindings.size()),
		layoutBindings.data()
	};

	if (device.vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Error during descriptor set layout creation" << std::endl;
	}

	VkPipelineLayoutCreateInfo layoutInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		1,
		&m_descriptorSetLayout,
		0,
		nullptr
	};

	if (device.vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Error during pipeline layout creation" << std::endl;
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr,
		0,
		2,
		pipelineShaderStages,
		&vertexInputInfo,
		&inputAssemblyInfo,
		nullptr,
		&viewportInfo,
		&rasterizationInfo,
		&multisampleInfo,
		&depthStencilInfo,
		&blendInfo,
		&dynamicInfo,
		m_pipelineLayout,
		rpass,
		0,
		VK_NULL_HANDLE,
		0
	};

	if (device.vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
	{
		std::cout << "Error during diffuse pipeline creation" << std::endl;
	}

}

CVulkanPipeline::~CVulkanPipeline()
{
	auto& device = CVulkanDevice::get();

	if (m_pipelineLayout)
	{
		device.vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
	}

	if (m_pipeline)
	{
		device.vkDestroyPipeline(device, m_pipeline, nullptr);
	}

	if (m_descriptorSetLayout)
	{
		device.vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	}

}

VkFormat CVulkanPipeline::attributeParamToVertFormat(SVertexAttribParams& p)
{
	switch (p.format)
	{
		case eFloat:
			switch (p.components)
			{
				case 4:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case 3:
					return VK_FORMAT_R32G32B32_SFLOAT;
			}
		case e1010102int:
			return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
	}

	return VK_FORMAT_UNDEFINED;
}

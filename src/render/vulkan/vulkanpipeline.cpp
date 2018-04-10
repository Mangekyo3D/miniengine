#include "vulkanpipeline.h"
#include "vulkanshadermodule.h"
#include "vulkandevice.h"
#include "vulkanrenderpass.h"
#include <string>
#include <array>
#include <vector>
#include <iostream>

CVulkanPipeline::CVulkanPipeline(SPipelineParams& params)
	: m_globaLayout(VK_NULL_HANDLE)
	, m_perDrawLayout(VK_NULL_HANDLE)
{
	auto& device = CVulkanDevice::get();

	for (auto samplerParams : params.samplers)
	{
		VkSampler sampler;
		VkSamplerCreateInfo samplerInfo = {
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			nullptr,
			0,
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			0.0f,
			VK_FALSE,
			0.0f,
			VK_FALSE,
			VK_COMPARE_OP_ALWAYS,
			0.0f,
			100.0f,
			VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
			VK_FALSE
		};

		if (device.vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			std::cout << "Failed to create sampler" << std::endl;
		}
		m_samplers.push_back(sampler);
	}

	std::string filename = params.shaderModule;
	CVulkanShaderModule fragmentShader(filename + ".frag.spv");
	CVulkanShaderModule vertexShader(filename + ".vert.spv");
	CVulkanRenderPass& rpass = static_cast <CVulkanRenderPass&> (*params.renderpass);

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
		((params.flags & ePrimitiveRestart) != 0)
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

	if (params.perVertBinding)
	{
		vertexBindings[numVertBindings++] =
			VkVertexInputBindingDescription {
				0,
				static_cast<uint32_t> (params.perVertBinding->dataSize),
				VK_VERTEX_INPUT_RATE_VERTEX
			};

		for (auto& attribs : params.perVertBinding->attributeParams)
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

	if (params.perInstanceBinding)
	{
		vertexBindings[numVertBindings++] =
			VkVertexInputBindingDescription {
				1,
				static_cast<uint32_t> (params.perInstanceBinding->dataSize),
				VK_VERTEX_INPUT_RATE_INSTANCE
			};

		for (auto& attribs : params.perInstanceBinding->attributeParams)
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
		static_cast <VkCullModeFlags> ((params.flags & eCullBackFace) ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE),
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
	bool bDepthWrite = VK_FALSE;

	if (params.flags & eDepthCompareGreater)
	{
		depthCompOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
		bDepthCompare = VK_TRUE;
		bDepthWrite = VK_TRUE;
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

	uint32_t numLayouts = 0;
	VkDescriptorSetLayout setLayouts[2];

	if ((setLayouts[numLayouts] = createSet(params.globalSet)) != VK_NULL_HANDLE)
	{
		m_globaLayout = setLayouts[numLayouts++];
	}
	if ((setLayouts[numLayouts] = createSet(params.perDrawSet)) != VK_NULL_HANDLE)
	{
		m_perDrawLayout = setLayouts[numLayouts++];
	}

	VkPipelineLayoutCreateInfo layoutInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		numLayouts,
		setLayouts,
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
		std::cout << "Error during " << params.shaderModule << " pipeline creation" << std::endl;
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

	if (m_globaLayout)
	{
		device.vkDestroyDescriptorSetLayout(device, m_globaLayout, nullptr);
	}

	if (m_perDrawLayout)
	{
		device.vkDestroyDescriptorSetLayout(device, m_perDrawLayout, nullptr);
	}

	for (auto& sampler : m_samplers)
	{
		device.vkDestroySampler(device, sampler, nullptr);
	}
}

void CVulkanPipeline::setRequiredPerFrameDescriptors(size_t numDescriptors)
{

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

VkShaderStageFlags CVulkanPipeline::stageFlagsToVulkanFlags(uint32_t stages)
{
	VkShaderStageFlags flags = 0;

	if (stages & eVertexStage)
	{
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (stages & eFragmentStage)
	{
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	return flags;
}

VkDescriptorType CVulkanPipeline::descriptorToVulkanType(EDescriptorType desc)
{
	switch (desc)
	{
		case eUniformBlock:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case eTextureSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

VkDescriptorSetLayout CVulkanPipeline::createSet(SDescriptorSet* params)
{
	if (params)
	{
		auto& device = CVulkanDevice::get();

		std::vector <VkDescriptorSetLayoutBinding> layoutBindings;

		for (auto& descriptor : params->descriptors)
		{
			layoutBindings.push_back(
				VkDescriptorSetLayoutBinding {
					static_cast <uint32_t> (layoutBindings.size()),
					descriptorToVulkanType(descriptor.type),
					1,
					stageFlagsToVulkanFlags(descriptor.shaderStages),
					(descriptor.type == eTextureSampler) ? &m_samplers[descriptor.sampler] : nullptr
				});
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			static_cast <uint32_t> (layoutBindings.size()),
			layoutBindings.data()
		};

		VkDescriptorSetLayout setLayout;

		if (device.vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &setLayout) != VK_SUCCESS)
		{
			std::cout << "Error during descriptor set layout creation" << std::endl;
		}

		return setLayout;
	}

	return VK_NULL_HANDLE;
}

SDescriptorPool::SDescriptorPool(VkDescriptorPoolSize* perDesrInfo, uint32_t numDecr)
{
	auto& device = CVulkanDevice::get();

	std::array <VkDescriptorPoolSize, 1> poolSizes {
		VkDescriptorPoolSize {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1
		}
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		0,
		1,
		numDecr,
		perDesrInfo
	};

	if (device.vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_pool) != VK_SUCCESS)
	{
		std::cout << "Error during descriptor pool creation" << std::endl;
	}

}

SDescriptorPool::~SDescriptorPool()
{
	auto& device = CVulkanDevice::get();

	if (m_pool)
	{
		device.vkDestroyDescriptorPool(device, m_pool, nullptr);
	}
}

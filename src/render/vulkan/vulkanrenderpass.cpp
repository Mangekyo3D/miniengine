#include "vulkanrenderpass.h"
#include "vulkandevice.h"
#include "vulkantexture.h"
#include "vulkandescriptorset.h"
#include <vector>
#include <iostream>
#include <cassert>

CVulkanRenderPass::CVulkanRenderPass(SRenderPassParams& params)
	: m_renderPass(VK_NULL_HANDLE)
	, m_framebuffer(VK_NULL_HANDLE)
	, bIsSwapchainPass(false)
	, m_b3DPass(params.b3DPass)
{
	if (params.set)
	{
		m_descriptorSet = std::make_unique <CVulkanDescriptorSet> (*params.set);
	}
}

CVulkanRenderPass::~CVulkanRenderPass()
{
	auto& device = CVulkanDevice::get();
	if (m_framebuffer)
	{
		device.vkDestroyFramebuffer(device, m_framebuffer, nullptr);
	}

	if (m_renderPass)
	{
		device.vkDestroyRenderPass(device, m_renderPass, nullptr);
	}
}

void CVulkanRenderPass::ensureRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	if (m_renderPass != VK_NULL_HANDLE)
	{
		return;
	}

	std::vector <VkAttachmentDescription> attachmentDescription;
	std::vector <VkAttachmentReference> attachmentReferences;
	VkAttachmentReference depthReference;
	attachmentDescription.reserve(numOutputs);
	attachmentReferences.reserve(numOutputs);

	auto& device = CVulkanDevice::get();

	if (numOutputs == 0 && !depthOut)
	{
		// we are writing to the swapchain color buffer in that case, prepare the renderpass accordingly
		VkAttachmentDescription description {
			0,
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference reference {
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		attachmentDescription.push_back(description);
		attachmentReferences.push_back(reference);
	}

	for (uint32_t i = 0; i < numOutputs; ++i)
	{
		CVulkanTexture* tex = static_cast <CVulkanTexture*> (outputs[i]);

		assert((tex->getUsage() & ITexture::eAttachement) != 0);

		VkAttachmentStoreOp storeOp = ((tex->getUsage() & ITexture::eSampled) != 0) ?
					VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentDescription description {
			0,
			tex->typeToFormat(),
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			storeOp,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkAttachmentReference reference {
			i,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		attachmentDescription.push_back(description);
		attachmentReferences.push_back(reference);
	}

	std::vector <VkSubpassDependency> dependencies =
	{
		VkSubpassDependency {
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},

		VkSubpassDependency {
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
	};

	if (depthOut)
	{
		CVulkanTexture* tex = static_cast <CVulkanTexture*> (depthOut);

		assert((tex->getUsage() & ITexture::eAttachement) != 0);

		VkAttachmentStoreOp storeOp = ((tex->getUsage() & ITexture::eSampled) != 0) ?
					VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentDescription description {
			0,
			tex->typeToFormat(),
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			storeOp,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		depthReference = {
			static_cast <uint32_t> (attachmentDescription.size()),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		attachmentDescription.push_back(description);

		dependencies.push_back(VkSubpassDependency {
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		});

		dependencies.push_back(VkSubpassDependency {
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		});
	}

	VkSubpassDescription subpassDescription[] = {
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,
			nullptr,
			static_cast<uint32_t> (attachmentReferences.size()),
			attachmentReferences.data(),
			nullptr,
			depthOut ? &depthReference : nullptr,
			0,
			nullptr
		}
	};

	VkRenderPassCreateInfo renderPassInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		static_cast <uint32_t> (attachmentDescription.size()),
		attachmentDescription.data(),
		1,
		subpassDescription,
		static_cast <uint32_t> (dependencies.size()),
		dependencies.data()
	};

	if (device.vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
	{
		std::cout << "failed to create renderpass" << std::endl;
	}
}

void CVulkanRenderPass::setupFrameBuffer(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	auto& device = CVulkanDevice::get();

	if (numOutputs == 0 && !depthOut)
	{
		bIsSwapchainPass = true;
		return;
	}

	if (m_framebuffer)
	{
		device.vkDestroyFramebuffer(device, m_framebuffer, nullptr);
	}

	std::vector <VkImageView> imageViews;
	imageViews.reserve(numOutputs + (depthOut ? 1 : 0));

	for (uint32_t i = 0; i < numOutputs; ++i)
	{
		CVulkanTexture* tex = static_cast <CVulkanTexture*> (outputs[i]);

		imageViews.push_back(tex->getView());
	}

	if (depthOut)
	{
		CVulkanTexture* tex = static_cast <CVulkanTexture*> (depthOut);

		imageViews.push_back(tex->getView());
	}

	m_width = outputs[0]->getWidth();
	m_height = outputs[0]->getHeight();

	VkFramebufferCreateInfo framebufferCreateInfo = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,
		0,
		m_renderPass,
		static_cast <uint32_t> (imageViews.size()),
		imageViews.data(),
		m_width,
		m_height,
		1
	};

	if (device.vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create framebuffer" << std::endl;
	}
}

// note, vulkan renderpasses should always be setup with the same number and kind of outputs
void CVulkanRenderPass::setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	ensureRenderPass(outputs, numOutputs, depthOut);
	setupFrameBuffer(outputs, numOutputs, depthOut);
}

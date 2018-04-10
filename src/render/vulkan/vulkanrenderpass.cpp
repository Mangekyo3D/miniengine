#include "vulkanrenderpass.h"
#include "vulkandevice.h"
#include "vulkantexture.h"
#include <vector>
#include <iostream>
#include <cassert>

CVulkanRenderPass::CVulkanRenderPass()
	: m_renderPass(VK_NULL_HANDLE)
	, m_framebuffer(VK_NULL_HANDLE)
	, bIsSwapchainPass(false)
{
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
			VK_ATTACHMENT_LOAD_OP_CLEAR,
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
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference reference {
			i,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		attachmentDescription.push_back(description);
		attachmentReferences.push_back(reference);
	}

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
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			storeOp,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		depthReference = {
			static_cast <uint32_t> (attachmentDescription.size()),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		attachmentDescription.push_back(description);
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
		0,
		nullptr
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

#include "vulkancommandbuffer.h"
#include "vulkandevice.h"
#include "vulkanswapchain.h"
#include "vulkanpipeline.h"
#include "vulkanbuffer.h"
#include "vulkanrenderpass.h"
#include "vulkantexture.h"

#include <iostream>

CVulkanCommandBuffer::CVulkanCommandBuffer(ISwapchain& swapchain)
	: m_device(&CVulkanDevice::get())
	, m_swapchain(static_cast<CVulkanSwapchain*> (&swapchain))
	, m_frame(&m_swapchain->getNextFrame())
	, m_cmd(m_frame->m_commandBuffer)
	, m_streamingBuffer(nullptr)
{
	m_device->vkResetCommandBuffer(m_cmd, 0);

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		nullptr
	};

	if (m_device->vkBeginCommandBuffer(m_cmd, &cmdBufferBeginInfo) != VK_SUCCESS)
	{
		std::cout << "Command Buffer Failed to initialize" << std::endl;
	}

	if (m_device->getGraphicsQueue() != m_device->getPresentQueue())
	{
		VkImageSubresourceRange subresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};

		VkImageMemoryBarrier toClear = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			0,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			m_device->getPresentQueueIndex(),
			m_device->getGraphicsQueueIndex(),
			m_frame->m_swapchainImage,
			subresourceRange
		};

		m_device->vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toClear);
	}
}

CVulkanCommandBuffer::~CVulkanCommandBuffer()
{
	if (m_device->getGraphicsQueue() != m_device->getPresentQueue())
	{
		VkImageSubresourceRange subresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};

		VkImageMemoryBarrier toPresent = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			m_device->getGraphicsQueueIndex(),
			m_device->getPresentQueueIndex(),
			m_frame->m_swapchainImage,
			subresourceRange
		};

		m_device->vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &toPresent);
	}

	if (m_device->vkEndCommandBuffer(m_cmd) != VK_SUCCESS)
	{
		std::cout << "Command Buffer Failed to record" << std::endl;
	}

	VkPipelineStageFlags waitDstMasks = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		1,
		&m_frame->m_swapchainImageAvailableSemaphore,
		&waitDstMasks,
		1,
		&m_cmd,
		1,
		&m_frame->m_renderingFinishedSemaphore
	};

	// submit all commands in queue
	if (m_device->vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, m_frame->m_fence) != VK_SUCCESS)
	{
		std::cout << "Error during queue submit" << std::endl;
	}

	m_frame->orphanBuffer(std::move(m_streamingBuffer));
}

IGPUBuffer& CVulkanCommandBuffer::createStreamingBuffer(size_t size)
{
	auto newBuf = m_device->createGPUBuffer(size, IGPUBuffer::Usage::eStreamSource);
	m_streamingBuffer.reset(static_cast<CVulkanBuffer*> (newBuf.release()));
	return *m_streamingBuffer;
}

void CVulkanCommandBuffer::copyBufferToTex(ITexture* tex, size_t offset,
										   uint16_t width, uint16_t height, uint8_t miplevel)
{
	CVulkanTexture* vkTex = static_cast<CVulkanTexture*> (tex);

	VkImageSubresourceRange subresourceRange = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		miplevel,
		1,
		0,
		1
	};

	VkImageMemoryBarrier toFill = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		m_device->getGraphicsQueueIndex(),
		m_device->getGraphicsQueueIndex(),
		*vkTex,
		subresourceRange
	};

	VkImageMemoryBarrier toUse = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		m_device->getGraphicsQueueIndex(),
		m_device->getGraphicsQueueIndex(),
		*vkTex,
		subresourceRange
	};

	m_device->vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toFill);

	VkImageSubresourceLayers subresource = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		miplevel,
		0,
		1
	};

	VkOffset3D imageOffset = {0, 0, 0};
	VkExtent3D imageExtent = {width, height, 1};
	VkBufferImageCopy copy {
		offset,
		0,
		0,
		subresource,
		imageOffset,
		imageExtent
	};

	m_device->vkCmdCopyBufferToImage(m_cmd,
				*m_streamingBuffer,
				*vkTex,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copy);

	m_device->vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toUse);
}

void CVulkanCommandBuffer::bindPipeline(IPipeline* pipeline)
{
	CVulkanPipeline* pipe = static_cast <CVulkanPipeline*> (pipeline);

	m_device->vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipe);
}

void CVulkanCommandBuffer::setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer,  IGPUBuffer* indexBuffer, bool bShortIndex)
{
	uint32_t bindCount = 0;
	VkBuffer buffers[2] = {0};
	VkDeviceSize offsets[2] = {0};

	CVulkanBuffer* vBuf = static_cast<CVulkanBuffer*> (vertexBuffer);
	buffers[bindCount] = *vBuf;
	offsets[bindCount] = vBuf->getAnimatedOffset();

	if (instanceBuffer)
	{
		++bindCount;
		CVulkanBuffer* vInst = static_cast<CVulkanBuffer*> (instanceBuffer);
		buffers[bindCount] = *vInst;
		offsets[bindCount] = vInst->getAnimatedOffset();
	}

	m_device->vkCmdBindVertexBuffers(m_cmd, 0, bindCount + 1, buffers, offsets);

	if (indexBuffer)
	{
		CVulkanBuffer* vInd = static_cast<CVulkanBuffer*> (indexBuffer);
		m_device->vkCmdBindIndexBuffer(m_cmd, *vInd, 0, bShortIndex ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	}
}

void CVulkanCommandBuffer::drawIndexedInstanced(EPrimitiveType type, size_t numIndices, size_t offset, size_t numInstances)
{
	m_device->vkCmdDrawIndexed(m_cmd, static_cast<uint32_t> (numIndices), static_cast<uint32_t> (numInstances), 0, static_cast<uint32_t> (offset), 0);
}

void CVulkanCommandBuffer::drawArrays(EPrimitiveType type, uint32_t start, uint32_t count)
{
	m_device->vkCmdDraw(m_cmd, count, 1, start, 0);
}

IDevice& CVulkanCommandBuffer::getDevice()
{
	return *m_device;
}

void CVulkanCommandBuffer::beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth)
{
	CVulkanRenderPass& rpass = static_cast<CVulkanRenderPass&> (renderpass);

	// initialize data - clear colors, viewport size
	VkClearValue clearValues[2];

	uint32_t clearComponents = 0;
	if (vClearColor)
	{
		clearValues[clearComponents].color.float32[0] = vClearColor[0];
		clearValues[clearComponents].color.float32[1] = vClearColor[1];
		clearValues[clearComponents].color.float32[2] = vClearColor[2];
		clearValues[clearComponents].color.float32[3] = vClearColor[3];

		clearComponents++;
	}
	if (clearDepth)
	{
		clearValues[clearComponents].depthStencil = {*clearDepth, 0};

		clearComponents++;
	}

	VkExtent2D extent;
	VkFramebuffer framebuffer;

	if (rpass.isSwapchainPass())
	{
		extent = m_swapchain->getExtent();

		if (m_frame->m_framebuffer != VK_NULL_HANDLE)
		{
			m_device->vkDestroyFramebuffer(*m_device, m_frame->m_framebuffer, nullptr);
			m_frame->m_framebuffer = VK_NULL_HANDLE;
		}

		VkFramebufferCreateInfo framebufferCreateInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			rpass,
			1,
			&m_frame->m_imageView,
			extent.width,
			extent.height,
			1
		};

		if (m_device->vkCreateFramebuffer(*m_device, &framebufferCreateInfo, nullptr, &m_frame->m_framebuffer) != VK_SUCCESS)
		{
			std::cout << "Failed to create framebuffer" << std::endl;
		}

		framebuffer = m_frame->m_framebuffer;
	}
	else
	{
		extent.width = rpass.getWidth();
		extent.height = rpass.getHeight();

		framebuffer = rpass.getFramebuffer();
	}

	VkRect2D rect = {VkOffset2D {0, 0}, extent};

	VkViewport viewportBounds = {
		0.0f,
		0.0f,
		static_cast <float> (extent.width),
		static_cast <float> (extent.height),
		1.0f,
		0.0f
	};

	VkRenderPassBeginInfo renderPassBeginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		nullptr,
		rpass,
		framebuffer,
		rect,
		clearComponents,
		clearValues
	};

	m_device->vkCmdBeginRenderPass(m_cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_device->vkCmdSetViewport(m_cmd, 0, 1, &viewportBounds);
	m_device->vkCmdSetScissor(m_cmd, 0, 1, &rect);
}

void CVulkanCommandBuffer::endRenderPass()
{
	// the end. Submit and chill
	m_device->vkCmdEndRenderPass(m_cmd);
}


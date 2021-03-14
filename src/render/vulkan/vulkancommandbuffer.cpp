#include "vulkancommandbuffer.h"
#include "vulkandevice.h"
#include "vulkanswapchain.h"
#include "vulkanpipeline.h"
#include "vulkanbuffer.h"
#include "vulkanrenderpass.h"
#include "vulkantexture.h"
#include "vulkandescriptorset.h"

#include <iostream>

CVulkanCommandBuffer::CVulkanCommandBuffer(ISwapchain& swapchain)
	: m_device(CVulkanDevice::get())
	, m_swapchain(static_cast<CVulkanSwapchain*> (&swapchain))
	, m_currentPipelineLayout(VK_NULL_HANDLE)
	, m_frame(&m_swapchain->getNextFrame())
	, m_cmd(m_frame->m_commandBuffer)
	, m_renderPassGlobalSet(VK_NULL_HANDLE)
	, m_pipelineGlobalSet(VK_NULL_HANDLE)
{
	m_device.vkResetCommandBuffer(m_cmd, 0);

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		nullptr
	};

	if (m_device.vkBeginCommandBuffer(m_cmd, &cmdBufferBeginInfo) != VK_SUCCESS)
	{
		std::cout << "Command Buffer Failed to initialize" << std::endl;
	}

	if (m_device.getGraphicsQueue() != m_device.getPresentQueue())
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
			m_device.getPresentQueueIndex(),
			m_device.getGraphicsQueueIndex(),
			m_frame->m_swapchainImage,
			subresourceRange
		};

		m_device.vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toClear);
	}
}

CVulkanCommandBuffer::~CVulkanCommandBuffer()
{
	if (m_device.getGraphicsQueue() != m_device.getPresentQueue())
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
			m_device.getGraphicsQueueIndex(),
			m_device.getPresentQueueIndex(),
			m_frame->m_swapchainImage,
			subresourceRange
		};

		m_device.vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &toPresent);
	}

	if (m_device.vkEndCommandBuffer(m_cmd) != VK_SUCCESS)
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
	if (m_device.vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, m_frame->m_fence) != VK_SUCCESS)
	{
		std::cout << "Error during queue submit" << std::endl;
	}

	m_frame->orphanBuffer(std::move(m_streamingBuffer));
	m_frame->orphanDescriptorPool(std::move(m_pipelineGlobalPool));
	m_frame->orphanDescriptorPool(std::move(m_pipelinePerDrawPool));
	m_frame->orphanDescriptorPool(std::move(m_renderpassGlobalPool));
}

IGPUBuffer& CVulkanCommandBuffer::createStreamingBuffer(size_t size)
{
	auto newBuf = m_device.createGPUBuffer(size, IGPUBuffer::Usage::eStreamSource);
	m_streamingBuffer.reset(static_cast<CVulkanBuffer*> (newBuf.release()));
	return *m_streamingBuffer;
}

void CVulkanCommandBuffer::copyBufferToTex(ITexture* tex, size_t offset,
										   uint32_t width, uint32_t height, uint8_t miplevel)
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
		VK_ACCESS_HOST_WRITE_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		m_device.getGraphicsQueueIndex(),
		m_device.getGraphicsQueueIndex(),
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
		m_device.getGraphicsQueueIndex(),
		m_device.getGraphicsQueueIndex(),
		*vkTex,
		subresourceRange
	};

	m_device.vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toFill);

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

	m_device.vkCmdCopyBufferToImage(m_cmd,
				*m_streamingBuffer,
				*vkTex,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copy);

	m_device.vkCmdPipelineBarrier(m_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toUse);
}

void CVulkanCommandBuffer::bindPipeline(IPipeline* pipeline, size_t numRequiredDescriptors)
{
	CVulkanPipeline* pipe = static_cast <CVulkanPipeline*> (pipeline);

	m_device.vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipe);

	m_frame->orphanDescriptorPool(std::move(m_pipelineGlobalPool));
	m_frame->orphanDescriptorPool(std::move(m_pipelinePerDrawPool));

	if (CVulkanDescriptorSet* globalSet = pipe->getGlobalSet())
	{
		m_pipelineGlobalPool = globalSet->createDescriptorPool(1);
		++m_perDrawSetIndex;
	}
	if (CVulkanDescriptorSet* perDrawSet = pipe->getPerDrawSet())
		m_pipelinePerDrawPool  = perDrawSet->createDescriptorPool(static_cast <uint32_t> (numRequiredDescriptors));

	m_currentPipelineLayout = pipe->getLayout();

	m_pipelineGlobalSet = VK_NULL_HANDLE;
}

void CVulkanCommandBuffer::setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer,  IGPUBuffer* indexBuffer, bool bShortIndex)
{
	uint32_t bindCount = 0;
    VkBuffer buffers[2] = {nullptr, nullptr};
	VkDeviceSize offsets[2] = {0};

	CVulkanBuffer* vBuf = static_cast<CVulkanBuffer*> (vertexBuffer);
	buffers[bindCount] = *vBuf;
	offsets[bindCount] = vBuf->getAnimatedOffset();
	++bindCount;

	if (instanceBuffer)
	{
		CVulkanBuffer* vInst = static_cast<CVulkanBuffer*> (instanceBuffer);
		buffers[bindCount] = *vInst;
		offsets[bindCount] = vInst->getAnimatedOffset();
		++bindCount;
	}

	m_device.vkCmdBindVertexBuffers(m_cmd, 0, bindCount, buffers, offsets);

	if (indexBuffer)
	{
		CVulkanBuffer* vInd = static_cast<CVulkanBuffer*> (indexBuffer);
		m_device.vkCmdBindIndexBuffer(m_cmd, *vInd, 0, bShortIndex ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	}
}

void CVulkanCommandBuffer::drawIndexedInstanced(size_t numIndices, size_t offset, size_t numInstances)
{
	m_device.vkCmdDrawIndexed(m_cmd, static_cast<uint32_t> (numIndices), static_cast<uint32_t> (numInstances), 0, static_cast<int32_t> (offset), 0);
}

void CVulkanCommandBuffer::drawArrays(uint32_t start, uint32_t count)
{
	m_device.vkCmdDraw(m_cmd, count, 1, start, 0);
}

void CVulkanCommandBuffer::bindGlobalRenderPassDescriptors(size_t numBindings, SDescriptorSource* sources)
{
	if (m_renderPassGlobalSet == VK_NULL_HANDLE)
		m_renderPassGlobalSet = updateDescriptorsGeneric(*m_renderpassGlobalPool, numBindings, sources);

	m_device.vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipelineLayout, 0, 1, &m_renderPassGlobalSet, 0, nullptr);
}

void CVulkanCommandBuffer::bindGlobalPipelineDescriptors(size_t numBindings, SDescriptorSource* sources)
{
	if (m_pipelineGlobalSet == VK_NULL_HANDLE)
		m_pipelineGlobalSet = updateDescriptorsGeneric(*m_pipelineGlobalPool, numBindings, sources);

	m_device.vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipelineLayout, m_pipelineSetIndex, 1, &m_pipelineGlobalSet, 0, nullptr);
}

void CVulkanCommandBuffer::bindPerDrawDescriptors(size_t numBindings, SDescriptorSource* sources)
{
	VkDescriptorSet set = updateDescriptorsGeneric(*m_pipelinePerDrawPool, numBindings, sources);
	m_device.vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipelineLayout, m_perDrawSetIndex, 1, &set, 0, nullptr);
}

VkDescriptorSet CVulkanCommandBuffer::updateDescriptorsGeneric(CDescriptorPool& pool, size_t numBindings, SDescriptorSource* sources)
{
	struct DescriptorInfo
	{
		DescriptorInfo(VkDescriptorImageInfo info)
		{
			polymorphic.imageInfo = info;
		}

		DescriptorInfo(VkDescriptorBufferInfo info)
		{
			polymorphic.bufferInfo = info;
		}

		union {
			VkDescriptorImageInfo imageInfo;
			VkDescriptorBufferInfo bufferInfo;
		} polymorphic;
	};

	VkDescriptorSet set = pool.allocate();

	std::vector <DescriptorInfo> info;
	info.reserve(numBindings);
	std::vector <VkWriteDescriptorSet> writeSets;
	writeSets.reserve(numBindings);

	for (uint32_t i = 0; i < numBindings; ++i)
	{
		switch (sources[i].type)
		{
			case SDescriptorSource::eBuffer:
			{
				CVulkanBuffer* buffer = static_cast <CVulkanBuffer*> (sources[i].data.buffer);
				info.push_back(buffer->getDescriptorBufferInfo());
				writeSets.push_back(VkWriteDescriptorSet {
										VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
										nullptr,
										set,
										i,
										0,
										1,
										VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
										nullptr,
										&info.back().polymorphic.bufferInfo,
										nullptr
									});
				break;
			}
			case SDescriptorSource::eTexture:
			{
				CVulkanTexture* texture = static_cast <CVulkanTexture*> (sources[i].data.texture);
				info.push_back(texture->getDescriptorImageInfo());
				writeSets.push_back(VkWriteDescriptorSet {
										VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
										nullptr,
										set,
										i,
										0,
										1,
										VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
										&info.back().polymorphic.imageInfo,
										nullptr,
										nullptr
									});
				break;
			}
		}
	}

	m_device.vkUpdateDescriptorSets(m_device, static_cast <uint32_t> (writeSets.size()), writeSets.data(), 0, nullptr);

	return set;
}

IDevice& CVulkanCommandBuffer::getDevice()
{
	return m_device;
}

void CVulkanCommandBuffer::beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth)
{
	CVulkanRenderPass& rpass = static_cast<CVulkanRenderPass&> (renderpass);

	m_frame->orphanDescriptorPool(std::move(m_renderpassGlobalPool));
	m_pipelineSetIndex = 0;
	m_perDrawSetIndex = 0;
	if (CVulkanDescriptorSet* set = rpass.getDescriptorSet())
	{
		m_renderpassGlobalPool = set->createDescriptorPool(1);
		++m_pipelineSetIndex;
		++m_perDrawSetIndex;
	}

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
			m_device.vkDestroyFramebuffer(m_device, m_frame->m_framebuffer, nullptr);
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

		if (m_device.vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &m_frame->m_framebuffer) != VK_SUCCESS)
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

	// depending on whether we have a projection transform, or 3D scene, we need to patch
	// the viewport transform accordingly
	bool bIs3DPass = rpass.is3DPass();
	float fHeight = static_cast <float> (extent.height);
	float startY = bIs3DPass ? fHeight : 0.0f;
	float Yoffset = bIs3DPass ? -fHeight : fHeight;

	VkViewport viewportBounds = {
		0.0f,
		startY,
		static_cast <float> (extent.width),
		Yoffset,
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

	m_device.vkCmdBeginRenderPass(m_cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_device.vkCmdSetViewport(m_cmd, 0, 1, &viewportBounds);
	m_device.vkCmdSetScissor(m_cmd, 0, 1, &rect);

	m_renderPassGlobalSet = VK_NULL_HANDLE;
}

void CVulkanCommandBuffer::endRenderPass()
{
	// the end. Submit and chill
	m_device.vkCmdEndRenderPass(m_cmd);
}


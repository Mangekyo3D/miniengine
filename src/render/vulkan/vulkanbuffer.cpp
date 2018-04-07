#include "vulkanbuffer.h"
#include "vulkandevice.h"
#include "vulkanswapchain.h"
#include <iostream>

CGPUBuffer::CGPUBuffer(size_t size, Usage usage)
	: m_lastUser(nullptr)
	, m_size(size)
	, m_usage(usage)
	, m_frame(0)
	, m_buffer(VK_NULL_HANDLE)
	, m_memoryChunk(nullptr)
{
	// if our size is less than the uniform offset alignment, make sure it fits
	if (m_usage == Usage::eAnimatedUniform)
	{
		auto& device = CVulkanDevice::get();
		VkPhysicalDevice vkPhysicalDevice = device.getPhysicalDevice();

		VkPhysicalDeviceProperties properties;
		device.vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);

		m_size = ((m_size + properties.limits.minUniformBufferOffsetAlignment - 1) / properties.limits.minUniformBufferOffsetAlignment)
				* properties.limits.minUniformBufferOffsetAlignment;
	}

	create();
}

void CGPUBuffer::create()
{
	auto& device = CVulkanDevice::get();

	size_t size = m_size;

	// if the buffer is animated, we will reserve space for each and every frame
	if (m_usage == Usage::eAnimatedUniform)
	{
		size *= CVulkanSwapchain::getNumberOfVirtualFrames();
	}

	VkBufferCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		size,
		toVulkanUsageFlags(m_usage),
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr
	};

	if (device.vkCreateBuffer(device, &createInfo, nullptr, &m_buffer) != VK_SUCCESS)
	{
		std::cout << "Error, vertex buffer creation failed" << std::endl;
		return;
	}

	VkMemoryRequirements memoryRequirements;
	device.vkGetBufferMemoryRequirements(device, m_buffer, &memoryRequirements);

	if (!device.allocateMemory(&m_memoryChunk, m_offset, memoryRequirements))
	{
		std::cout << "Error, memory allocation failed" << std::endl;
		return;
	}

	if (device.vkBindBufferMemory(device, m_buffer, m_memoryChunk->m_memory, m_offset) != VK_SUCCESS)
	{
		std::cout << "Error, binding memory to vertex buffer creation" << std::endl;
	}
}

CGPUBuffer::~CGPUBuffer()
{
	auto& device = CVulkanDevice::get();

	if (m_buffer)
	{
		device.vkDestroyBuffer(device, m_buffer, nullptr);
	}

	m_memoryChunk->freeBlock(m_offset);
}

VkDescriptorBufferInfo CGPUBuffer::getDescriptorBufferInfo() const
{
	return {m_buffer, 0, m_size};
}

VkBufferUsageFlags CGPUBuffer::toVulkanUsageFlags(CGPUBuffer::Usage usage)
{
	switch(usage)
	{
		case CGPUBuffer::Usage::eConstantVertex:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case CGPUBuffer::Usage::eIndex:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case CGPUBuffer::Usage::eAnimatedUniform:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
}

void* CGPUBuffer::map()
{
	auto& device = CVulkanDevice::get();

	if (m_usage == Usage::eAnimatedUniform)
	{
		m_frame = (m_frame + 1) % CVulkanSwapchain::getNumberOfVirtualFrames();
	}

	void* mappedPointer = nullptr;
	if (device.vkMapMemory(device, m_memoryChunk->m_memory, m_offset + m_frame * m_size, m_size, 0, &mappedPointer) != VK_SUCCESS)
	{
		std::cout << "Failed to map memory" << std::endl;
	}

	return mappedPointer;
}


void CGPUBuffer::unmap()
{
	auto& device = CVulkanDevice::get();

	VkMappedMemoryRange mappedRange = {
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		nullptr,
		m_memoryChunk->m_memory,
		m_offset + m_frame * m_size,
		m_size
	};

	device.vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	device.vkUnmapMemory(device, m_memoryChunk->m_memory);
}

void CGPUBuffer::bindAsVertexBuffer(SFrame& frame)
{
	auto& device = CVulkanDevice::get();
	const VkDeviceSize offset = m_frame * m_size;
	VkCommandBuffer& cmd = frame.m_commandBuffer;

	device.vkCmdBindVertexBuffers(cmd, 0, 1, &m_buffer, &offset);
	m_lastUser = &frame;
}

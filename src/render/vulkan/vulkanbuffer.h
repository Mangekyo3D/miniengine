#pragma once
#include "../igpubuffer.h"
#include <vulkan/vulkan.h>

struct SMemoryChunk;

class CVulkanBuffer : public IGPUBuffer
{
public:
	CVulkanBuffer(size_t size, uint32_t usage);
	~CVulkanBuffer();

	VkDeviceSize getAnimatedOffset() const { return static_cast <uint32_t>(m_frame * m_size); }
	VkDescriptorBufferInfo getDescriptorBufferInfo() const;

	operator VkBuffer () {return m_buffer; }

private:
	void create();
	virtual void *lock() override;
	virtual void unlock() override;

	VkBufferUsageFlags toVulkanUsageFlags(uint32_t usage);
	// memory block - includes information about memory object, size and offset within this object

	VkBuffer m_buffer;
	// size of buffer
	size_t     m_offset;
	// stores current frame for animated buffers
	uint8_t    m_frame;

	SMemoryChunk* m_memoryChunk;
};

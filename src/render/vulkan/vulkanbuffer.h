#pragma once
#include "../igpubuffer.h"
#include <vulkan/vulkan.h>

struct SMemoryChunk;
struct SFrame;

class CVulkanBuffer : public IGPUBuffer
{
public:
	CVulkanBuffer(size_t size, Usage usage);
	~CVulkanBuffer();

	uint32_t getAnimatedOffset() { return static_cast <uint32_t>(m_frame * m_size); }
	VkDescriptorBufferInfo getDescriptorBufferInfo() const;

	void bindAsVertexBuffer(SFrame& frame);

private:
	void create();
	virtual void *lock() override;
	virtual void unlock() override;

	VkBufferUsageFlags toVulkanUsageFlags(IGPUBuffer::Usage usage);
	// memory block - includes information about memory object, size and offset within this object

	VkBuffer m_buffer;
	// size of buffer
	size_t     m_size;
	size_t     m_offset;
	// stores current frame for animated buffers
	uint8_t    m_frame;

	Usage      m_usage;
	SMemoryChunk* m_memoryChunk;

	// last user of this buffer. When the buffer is flushed, the frame is responsible for cleaning up the buffer
	SFrame* m_lastUser;
};

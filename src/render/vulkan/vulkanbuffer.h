#pragma once
#include <vulkan/vulkan.h>

struct SMemoryChunk;
struct SFrame;

class CGPUBuffer
{
public:

	// Helper class that wraps a buffer memory mapping operation
	template <class T> class CAutoMapper
	{
	public:
		CAutoMapper(CGPUBuffer& buffer)
			: m_buffer(buffer)
			, m_mappedPointer(nullptr)
		{
			m_mappedPointer = static_cast<T*> (buffer.map());
		}

		~CAutoMapper()
		{
			if (m_mappedPointer)
			{
				m_buffer.unmap();
			}
		}

		CAutoMapper(CAutoMapper&& other)
			: m_buffer(other.m_buffer)
		{
			m_mappedPointer = other.m_mappedPointer;
			other.m_mappedPointer = nullptr;
		}

		explicit operator bool() const
		{
			return m_mappedPointer != nullptr;
		}

		operator T* ()
		{
			return m_mappedPointer;
		}

		T* operator ->()
		{
			return m_mappedPointer;
		}

		T& operator [](size_t index)
		{
			return m_mappedPointer[index];
		}


	private:
		CGPUBuffer& m_buffer;
		T* m_mappedPointer;
	};

	enum class Usage {
		eConstantVertex, // Vertex buffer that never changes
		eIndex,
		eAnimatedUniform, // Uniform that changes potentially every frame
	};

	CGPUBuffer(size_t size, Usage usage);
	~CGPUBuffer();

	uint32_t getAnimatedOffset() { return static_cast <uint32_t>(m_frame * m_size); }
	VkDescriptorBufferInfo getDescriptorBufferInfo() const;

	void bindAsVertexBuffer(SFrame& frame);

private:
	void create();
	void *map();
	void unmap();

	VkBufferUsageFlags toVulkanUsageFlags(Usage usage);
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

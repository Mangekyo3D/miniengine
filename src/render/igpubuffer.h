#pragma once
#include <stddef.h>
#include <stdint.h>

class IGPUBuffer
{
public:
	enum Usage {
		eConstantVertex  =  1,       // Vertex buffer that never changes
		eIndex           = (1 << 1), // Index Buffer
		eAnimatedUniform = (1 << 2), // Uniform that changes potentially every frame
		eStreamSource    = (1 << 3)  // Source for streaming requests
	};

	IGPUBuffer(size_t size, uint32_t usage) : m_size(size), m_usage(usage) {}
	IGPUBuffer(const IGPUBuffer&) = delete;
	IGPUBuffer& operator = (const IGPUBuffer&) = delete;
	virtual ~IGPUBuffer() {}

	// convenience template class that locks a buffer and can be dereferenced to the constant buffer type
	// it has been instantiated from
	template <typename T> class CAutoLock
	{
	public:
		CAutoLock(IGPUBuffer& buffer)
		    : m_buffer(buffer)
		    , m_mapPtr(nullptr)
		{
			m_mapPtr = static_cast<T*> (buffer.lock());
		}

		CAutoLock(CAutoLock&& lock)
		    : m_buffer(lock.m_buffer)
		    , m_mapPtr(lock.m_mapPtr)
		{
			lock.m_mapPtr = nullptr;
		}

		~CAutoLock()
		{
			if (m_mapPtr)
			{
				m_buffer.unlock();
			}
		}

		explicit operator bool()
		{
			return m_mapPtr != nullptr;
		}

		operator T* ()
		{
			return m_mapPtr;
		}

		T* operator ->()
		{
			return m_mapPtr;
		}

		T& operator [](size_t index)
		{
			return m_mapPtr[index];
		}

	private:
		IGPUBuffer& m_buffer;
		T* m_mapPtr;
	};

protected:
	virtual void* lock() = 0;
	virtual void  unlock() = 0;

	size_t m_size;
	uint32_t m_usage;
};

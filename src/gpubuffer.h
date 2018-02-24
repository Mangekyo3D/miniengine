#pragma once
#include <stddef.h>
#include <stdint.h>

class IGPUBuffer
{
public:
	IGPUBuffer(size_t size);
	~IGPUBuffer();

	void* lock();
	void  unlock();
	// this should not be necessary, it's implementation detail
	unsigned int getID() {return m_ID;}

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

	private:
		IGPUBuffer& m_buffer;
		T* m_mapPtr;
	};

private:
	uint32_t m_ID;
	size_t m_size;
};

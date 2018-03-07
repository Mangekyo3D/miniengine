#pragma once
#include "../gpubuffer.h"
#include <stdint.h>

class COpenGLBuffer : public IGPUBuffer
{
public:
	COpenGLBuffer(size_t size);
	~COpenGLBuffer();

	virtual void* lock();
	virtual void  unlock();
	unsigned int getID() {return m_ID;}

private:
	uint32_t m_ID;
};

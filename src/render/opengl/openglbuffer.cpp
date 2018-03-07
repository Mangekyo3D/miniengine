#include "opengldevice.h"
#include "openglbuffer.h"
#include <stddef.h>

COpenGLBuffer::COpenGLBuffer(size_t size)
	: IGPUBuffer(size)
{
	auto& device = COpenGLDevice::get();

	device.glCreateBuffers(1, &m_ID);
	device.glNamedBufferStorage(m_ID, size, nullptr, GL_MAP_WRITE_BIT);
}

COpenGLBuffer::~COpenGLBuffer()
{
	auto& device = COpenGLDevice::get();

	device.glDeleteBuffers(1, &m_ID);
}

void* COpenGLBuffer::lock()
{
	auto& device = COpenGLDevice::get();

	return device.glMapNamedBufferRange(m_ID, 0, m_size, GL_MAP_WRITE_BIT);
}

void  COpenGLBuffer::unlock()
{
	auto& device = COpenGLDevice::get();

	device.glUnmapNamedBuffer(m_ID);
}

#include "opengldevice.h"
#include "openglbuffer.h"
#include <stddef.h>

COpenGLBuffer::COpenGLBuffer(size_t size, uint32_t usage)
	: IGPUBuffer(size, usage)
{
	auto& device = COpenGLDevice::get();

	device.glCreateBuffers(1, &m_ID);
	device.glNamedBufferStorage(m_ID, static_cast<GLsizeiptr> (size), nullptr, GL_MAP_WRITE_BIT);
}

COpenGLBuffer::~COpenGLBuffer()
{
	auto& device = COpenGLDevice::get();

	device.glDeleteBuffers(1, &m_ID);
}

void* COpenGLBuffer::lock()
{
	auto& device = COpenGLDevice::get();

	return device.glMapNamedBufferRange(m_ID, 0, static_cast<GLsizeiptr> (m_size), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
}

void  COpenGLBuffer::unlock()
{
	auto& device = COpenGLDevice::get();

	device.glUnmapNamedBuffer(m_ID);
}

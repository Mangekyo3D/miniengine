#include "gpubuffer.h"
#include "opengldevice.h"

IGPUBuffer::IGPUBuffer(size_t size)
	: m_size(size)
{
	auto& device = IDevice::get <COpenGLDevice>();

	device.glCreateBuffers(1, &m_ID);
	device.glNamedBufferStorage(m_ID, size, nullptr, GL_MAP_WRITE_BIT);
}

IGPUBuffer::~IGPUBuffer()
{
	auto& device = IDevice::get <COpenGLDevice>();

	device.glDeleteBuffers(1, &m_ID);
}

void* IGPUBuffer::lock()
{
	auto& device = IDevice::get <COpenGLDevice>();

	return device.glMapNamedBufferRange(m_ID, 0, m_size, GL_MAP_WRITE_BIT);
}

void  IGPUBuffer::unlock()
{
	auto& device = IDevice::get <COpenGLDevice>();

	device.glUnmapNamedBuffer(m_ID);
}

#include "gpubuffer.h"
#include "cdevice.h"

CGPUBuffer::CGPUBuffer(size_t size)
	: m_size(size)
{
	auto& device = IDevice::get <CDevice>();

	device.glCreateBuffers(1, &m_ID);
	device.glNamedBufferStorage(m_ID, size, nullptr, GL_MAP_WRITE_BIT);
}

CGPUBuffer::~CGPUBuffer()
{
	auto& device = IDevice::get <CDevice>();

	device.glDeleteBuffers(1, &m_ID);
}

void* CGPUBuffer::lock()
{
	auto& device = IDevice::get <CDevice>();

	return device.glMapNamedBufferRange(m_ID, 0, m_size, GL_MAP_WRITE_BIT);
}

void  CGPUBuffer::unlock()
{
	auto& device = IDevice::get <CDevice>();

	device.glUnmapNamedBuffer(m_ID);
}

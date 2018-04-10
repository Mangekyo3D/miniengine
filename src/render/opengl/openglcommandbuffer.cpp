#include "openglcommandbuffer.h"
#include "opengldevice.h"
#include "openglbuffer.h"
#include "opengltexture.h"
#include "openglpipeline.h"
#include "openglrenderpass.h"

COpenGLCommandBuffer::COpenGLCommandBuffer(uint32_t swapchainWidth, uint32_t swapchainHeight)
	: m_swapchainWidth(swapchainWidth)
	, m_swapchainHeight(swapchainHeight)
	, m_device(&COpenGLDevice::get())
	, m_currentPipeline(nullptr)
	, m_currentVertexDescriptor(nullptr)
	, m_bShortIndices(true)
	, m_numGlobalBuffers(0)
	, m_numGlobalTextures(0)
{
}

COpenGLCommandBuffer::~COpenGLCommandBuffer()
{
	// unbind streaming buffer so it may be cleaned up properly
	m_device->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

IGPUBuffer& COpenGLCommandBuffer::createStreamingBuffer(size_t size)
{
	auto newBuffer = m_device->createGPUBuffer(size, IGPUBuffer::Usage::eStreamSource);
	m_streamingBuffer.reset(static_cast<COpenGLBuffer*> (newBuffer.release()));

	m_device->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	m_device->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_streamingBuffer->getID());

	return *m_streamingBuffer;
}

void COpenGLCommandBuffer::copyBufferToTex(ITexture* tex, size_t offset, uint16_t width, uint16_t height, uint8_t miplevel)
{
	COpenGLTexture* glTex = static_cast<COpenGLTexture*> (tex);

	m_device->glTextureSubImage2D(glTex->getID(), miplevel, 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, ((uint8_t*)nullptr + offset));
}

void COpenGLCommandBuffer::bindPipeline(IPipeline* pipeline)
{
	COpenGLPipeline* pline = static_cast<COpenGLPipeline*> (pipeline);

	if (m_currentPipeline)
	{
		m_currentVertexDescriptor = nullptr;
	}

	m_currentPipeline = pline;

	if (pline)
	{
		m_currentVertexDescriptor = pline->bind();
	}
}

void COpenGLCommandBuffer::bindGlobalDescriptors(size_t numBindings, SDescriptorSource* sources)
{
	m_numGlobalBuffers = 0;
	m_numGlobalTextures = 0;

	for (uint32_t i = 0; i < numBindings; ++i)
	{
		switch (sources[i].type)
		{
			case SDescriptorSource::eBuffer:
			{
				COpenGLBuffer* buf = static_cast<COpenGLBuffer*> (sources[i].data.buffer);
				m_device->glBindBufferBase(GL_UNIFORM_BUFFER, m_numGlobalBuffers++, buf->getID());
				break;
			}
			case SDescriptorSource::eTexture:
			{
				COpenGLTexture* tex = static_cast<COpenGLTexture*> (sources[i].data.texture);
				tex->bind(m_numGlobalTextures++);
				break;
			}
		}
	}
}

void COpenGLCommandBuffer::bindPerDrawDescriptors(size_t numBindings, SDescriptorSource* sources)
{
	uint32_t numGlobalBuffers = m_numGlobalBuffers;
	uint32_t numGlobalTextures = m_numGlobalTextures;

	for (uint32_t i = 0; i < numBindings; ++i)
	{
		switch (sources[i].type)
		{
			case SDescriptorSource::eBuffer:
			{
				COpenGLBuffer* buf = static_cast<COpenGLBuffer*> (sources[i].data.buffer);
				m_device->glBindBufferBase(GL_UNIFORM_BUFFER, numGlobalBuffers++, buf->getID());
				break;
			}
			case SDescriptorSource::eTexture:
			{
				COpenGLTexture* tex = static_cast<COpenGLTexture*> (sources[i].data.texture);
				tex->bind(numGlobalTextures++);
				break;
			}
		}
	}
}

void COpenGLCommandBuffer::setVertexStream(IGPUBuffer* vertexBuffer, IGPUBuffer* instanceBuffer, IGPUBuffer* indexBuffer, bool bShortIndices)
{
	m_bShortIndices = bShortIndices;
	m_currentVertexDescriptor->setVertexStream(vertexBuffer, indexBuffer, instanceBuffer);
}

void COpenGLCommandBuffer::drawArrays(EPrimitiveType type, uint32_t start, uint32_t count)
{
	m_device->glDrawArrays(meshPrimitiveToGLPrimitive(type), start, count);
}

void COpenGLCommandBuffer::drawIndexedInstanced(EPrimitiveType type, size_t numIndices, size_t offset, size_t numInstances)
{
	m_device->glDrawElementsInstanced(meshPrimitiveToGLPrimitive(type),static_cast <GLint> (numIndices),
									  m_bShortIndices ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (static_cast <GLubyte*>(nullptr) + offset),
									  static_cast <GLint> (numInstances));
}

IDevice& COpenGLCommandBuffer::getDevice()
{
	return *m_device;
}


void COpenGLCommandBuffer::beginRenderPass(IRenderPass& renderpass, const float vClearColor[4], const float* clearDepth)
{
	COpenGLRenderPass& glpass = static_cast<COpenGLRenderPass&> (renderpass);
	uint32_t fbobjID = glpass.getID();
	m_device->glBindFramebuffer(GL_FRAMEBUFFER, fbobjID);

	if (fbobjID == 0)
	{
		m_device->glViewport(0, 0, m_swapchainWidth, m_swapchainHeight);
	}
	else
	{
		m_device->glViewport(0, 0, glpass.getWidth(), glpass.getHeight());
	}

	m_device->glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	m_device->glDepthRangef(1.0f, 0.0f);

	if (vClearColor)
	{
		// exception for zero framebuffer, it will have no outputs attached in OpenGL
		if (fbobjID == 0)
		{
			m_device->glClearNamedFramebufferfv(0, GL_COLOR, 0, vClearColor);
		}
		else
		{
			for (uint32_t i = 0; i < glpass.getNumOutputs(); ++i)
			{
				m_device->glClearNamedFramebufferfv(fbobjID, GL_COLOR, i, vClearColor);
			}
		}
	}

	if (clearDepth)
	{
		// exception for zero framebuffer, it will have no outputs attached in OpenGL
		if (fbobjID == 0)
		{
			m_device->glClearNamedFramebufferfi(0, GL_DEPTH_STENCIL, 0, *clearDepth, 0);
		}
		else if (glpass.hasDepthOutput())
		{
			m_device->glClearNamedFramebufferfi(fbobjID, GL_DEPTH_STENCIL, 0, *clearDepth, 0);
		}
	}
}

void COpenGLCommandBuffer::endRenderPass()
{

}

uint32_t COpenGLCommandBuffer::meshPrimitiveToGLPrimitive(EPrimitiveType type)
{
	switch (type)
	{
		case EPrimitiveType::eTriangles:
			return GL_TRIANGLES;
		case EPrimitiveType::eTriangleStrip:
			return GL_TRIANGLE_STRIP;
		default:
			break;
	}

	return GL_TRIANGLES;
}

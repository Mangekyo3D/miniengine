#include "openglrenderpass.h"
#include "opengldevice.h"
#include "opengltexture.h"
#include <iostream>

COpenGLRenderPass::COpenGLRenderPass()
	: m_framebufferObject(0)
	, m_width(0)
	, m_height(0)
	, m_numOutputs(0)
	, m_bDepthOutput(false)
{
}

COpenGLRenderPass::~COpenGLRenderPass()
{
	auto& device = COpenGLDevice::get();
	if (m_framebufferObject)
	{
		device.glDeleteFramebuffers(1, &m_framebufferObject);
	}
}

void COpenGLRenderPass::setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	if (outputs && numOutputs > 4)
	{
		std::cout << "Too many attachments!" << std::endl;
	}

	m_numOutputs = 0;
	m_bDepthOutput = false;
	if ((outputs && numOutputs > 0) || depthOut)
	{
		auto& device = COpenGLDevice::get();

		if (m_framebufferObject != 0)
		{
			device.glDeleteFramebuffers(1, &m_framebufferObject);
			m_framebufferObject = 0;
		}

		device.glCreateFramebuffers(1, &m_framebufferObject);

		if (outputs && numOutputs > 0)
		{
			uint32_t attachmentType = GL_COLOR_ATTACHMENT0;
			for (uint32_t i = 0; i < numOutputs; ++i)
			{
				COpenGLTexture* tex = static_cast <COpenGLTexture*> (outputs[i]);
				device.glNamedFramebufferTexture(m_framebufferObject, attachmentType, tex->getID(), 0);
				++attachmentType;
			}

			m_numOutputs = numOutputs;

			// ideally we must check if dimensions match
			m_width = outputs[0]->getWidth();
			m_height = outputs[0]->getHeight();
		}

		if (depthOut)
		{
			COpenGLTexture* tex = static_cast <COpenGLTexture*> (depthOut);
			device.glNamedFramebufferTexture(m_framebufferObject, GL_DEPTH_ATTACHMENT, tex->getID(), 0);

			// ideally we must check if dimensions match
			m_width = depthOut->getWidth();
			m_height = depthOut->getHeight();

			m_bDepthOutput = true;
		}
	}

}

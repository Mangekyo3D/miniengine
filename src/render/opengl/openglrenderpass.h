#pragma once
#include "../irenderpass.h"
#include <stdint.h>

class COpenGLRenderPass : public IRenderPass
{
	public:
		COpenGLRenderPass();
		~COpenGLRenderPass();

		// setup the renderpass with inputs and outputs.
		virtual void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut) override;

		uint32_t getNumOutputs() { return m_numOutputs; }
		bool     hasDepthOutput() {return m_bDepthOutput; }
		uint32_t getID() { return m_framebufferObject; }
		uint32_t getWidth() { return m_width; }
		uint32_t getHeight() { return m_height; }

	protected:
		uint32_t m_numOutputs;
		bool     m_bDepthOutput;

		// width and height, used when we render to default framebuffer
		uint32_t                m_width;
		uint32_t                m_height;

		// framebuffer object of this renderpass
		uint32_t m_framebufferObject;
};

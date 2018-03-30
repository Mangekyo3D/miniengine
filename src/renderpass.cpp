#include "renderpass.h"
#include "program.h"
#include "render/opengl/opengldevice.h"
#include "render/itexture.h"
#include "batch.h"
#include "resourcemanager.h"
#include "render/opengl/openglbuffer.h"
#include "render/opengl/opengltexture.h"
#include <iostream>

struct SFullScreenData
{
	SFullScreenData(COpenGLPipeline* pipeline, IDevice* device)
	{
		m_fullScreenTriangle = device->createGPUBuffer(3 * sizeof(VertexFormatV));

		if (auto lock = IGPUBuffer::CAutoLock <VertexFormatV>(*m_fullScreenTriangle))
		{
			VertexFormatV *v = lock;
			v[0].vertex = Vec3(-1.0, -1.0, 0.0);
			v[1].vertex = Vec3(3.0, -1.0, 0.0);
			v[2].vertex = Vec3(-1.0, 3.0, 0.0);
		}

		m_pipeline = pipeline;
	}

	std::unique_ptr <IGPUBuffer> m_fullScreenTriangle;
	COpenGLPipeline* m_pipeline;
};

CFullScreenRenderPass::CFullScreenRenderPass(COpenGLPipeline* pipeline, IDevice* device)
{
	m_data = std::make_unique <SFullScreenData> (pipeline, device);

	// create sampler for texture sampling of material
	auto& gldevice = COpenGLDevice::get();
	gldevice.glCreateSamplers(1, &m_sampler);
	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

CFullScreenRenderPass::~CFullScreenRenderPass()
{
	auto& device = COpenGLDevice::get();

	device.glDeleteSamplers(1, &m_sampler);
	m_sampler = 0;
}

void CFullScreenRenderPass::setupRenderPass(ITexture** inputs, uint32_t numInputs, uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	m_inputs.clear();
	m_inputs.reserve(numInputs);

	for (uint32_t i = 0; i < numInputs; ++i)
	{
		m_inputs.push_back(inputs[i]);
	}
}


void CFullScreenRenderPass::draw()
{
	auto& device = COpenGLDevice::get();

	device.glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
	device.glViewport(0, 0, m_width, m_height);

	IDescriptorInterface* desc = m_data->m_pipeline->bind();
	desc->setVertexStream(m_data->m_fullScreenTriangle.get(), nullptr, nullptr);

	for (int i = 0; i < m_inputs.size(); ++i)
	{
		m_inputs[i]->bind(i);
		device.glBindSampler(i, m_sampler);
	}

	device.glDrawArrays(GL_TRIANGLES, 0, 3);
}

CRenderPass::CRenderPass()
	: m_framebufferObject(0)
	, m_width(0)
	, m_height(0)
	, m_numOutputs(0)
	, m_bDepthOutput(false)
{
}

CRenderPass::~CRenderPass()
{
	auto& device = COpenGLDevice::get();
	if (m_framebufferObject)
	{
		device.glDeleteFramebuffers(1, &m_framebufferObject);
	}
}

void CRenderPass::setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
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

void CSceneRenderPass::draw(std::vector <std::unique_ptr<IBatch> > & batches, IGPUBuffer& cameraData, IGPUBuffer& lightData)
{
	auto& device = COpenGLDevice::get();

	static const float vClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	device.glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);

	device.glViewport(0, 0, m_width, m_height);

	for (uint32_t i = 0; i < m_numOutputs; ++i)
	{
		device.glClearNamedFramebufferfv(m_framebufferObject, GL_COLOR, i, vClearColor);
	}

	if (m_bDepthOutput)
	{
		device.glClearNamedFramebufferfi(m_framebufferObject, GL_DEPTH_STENCIL, 0, 0.0f, 0);
	}

	device.glEnable(GL_DEPTH_TEST);

	device.glDepthFunc(GL_GEQUAL);
	// inverse depth trick
	device.glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	device.glDepthRangef(1.0f, 0.0f);

	COpenGLBuffer& bglCameraData = static_cast<COpenGLBuffer&>(cameraData);
	COpenGLBuffer& bglLightData = static_cast<COpenGLBuffer&>(lightData);

	device.glBindBufferBase(GL_UNIFORM_BUFFER, 0, bglCameraData.getID());
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 1, bglLightData.getID());

	for (auto& batch : batches)
	{
		batch->draw();
	}

	device.glDisable(GL_DEPTH_TEST);
}

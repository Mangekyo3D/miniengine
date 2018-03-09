#include "renderpass.h"
#include "program.h"
#include "render/opengl/opengldevice.h"
#include "texture.h"
#include "batch.h"
#include "render/opengl/openglbuffer.h"
#include <iostream>

struct SFullScreenData
{
	SFullScreenData()
		: m_fullScreenTriangle(3 * sizeof(VertexFormatV))
	{
		if (auto lock = IGPUBuffer::CAutoLock <VertexFormatV>(m_fullScreenTriangle))
		{
			VertexFormatV *v = lock;
			v[0].vertex = Vec3(-1.0, -1.0, 0.0);
			v[1].vertex = Vec3(3.0, -1.0, 0.0);
			v[2].vertex = Vec3(-1.0, 3.0, 0.0);
		}

		m_fullScreenTriangleDescriptor.setVertexStream(&m_fullScreenTriangle, nullptr, nullptr);
	}

	ArrayDescriptorV m_fullScreenTriangleDescriptor;
	COpenGLBuffer    m_fullScreenTriangle;
	CProgram         m_program;
};

CFullScreenRenderPass::CFullScreenRenderPass(std::string shaderName)
{
	CShader vertShader(shaderName, CShader::EType::eVertex);
	CShader fragShader(shaderName, CShader::EType::eFragment);

	m_data = std::make_unique <SFullScreenData> ();

	m_data->m_program.attach(vertShader);
	m_data->m_program.attach(fragShader);

	m_data->m_program.link();

	// create sampler for texture sampling of material
	auto& device = COpenGLDevice::get();
	device.glCreateSamplers(1, &m_sampler);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

CFullScreenRenderPass::~CFullScreenRenderPass()
{
	auto& device = COpenGLDevice::get();

	if (m_framebufferObject)
	{
		device.glDeleteFramebuffers(1, &m_framebufferObject);
		m_framebufferObject = 0;
	}

	device.glDeleteSamplers(1, &m_sampler);
	m_sampler = 0;
}

void CFullScreenRenderPass::draw()
{
	auto& device = COpenGLDevice::get();

	device.glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
	device.glViewport(0, 0, m_width, m_height);

	m_data->m_program.use();

	m_data->m_fullScreenTriangleDescriptor.bind();

	for (int i = 0; i < m_inputs.size(); ++i)
	{
		m_inputs[i]->bind(i);
		device.glBindSampler(i, m_sampler);
	}

	device.glDrawArrays(GL_TRIANGLES, 0, 3);
}

void CRenderPass::addColorOutput(CTexture* tex)
{
	m_colorOutputs.push_back(tex);
	m_width = tex->getWidth();
	m_height = tex->getHeight();
}

void CRenderPass::addDepthOutput(CTexture* tex)
{
	m_depthOutput = tex;
	m_width = tex->getWidth();
	m_height = tex->getHeight();
}

void CRenderPass::resetOutputs()
{
	auto& device = COpenGLDevice::get();

	if (m_framebufferObject)
	{
		device.glDeleteFramebuffers(1, &m_framebufferObject);
		m_framebufferObject = 0;
	}

	m_colorOutputs.clear();
	m_depthOutput = nullptr;

	m_width = m_height = 0;
}

void CRenderPass::finalize()
{
	auto& device = COpenGLDevice::get();

	if (m_colorOutputs.size() > 4)
	{
		std::cout << "Too many attachments!" << std::endl;
	}

	if (m_colorOutputs.size() > 0)
	{
		device.glCreateFramebuffers(1, &m_framebufferObject);

		uint32_t attachmentType = GL_COLOR_ATTACHMENT0;
		for (auto tex : m_colorOutputs)
		{
			device.glNamedFramebufferTexture(m_framebufferObject, attachmentType, tex->getID(), 0);
			++attachmentType;
		}

		if (m_depthOutput)
		{
			device.glNamedFramebufferTexture(m_framebufferObject, GL_DEPTH_ATTACHMENT, m_depthOutput->getID(), 0);
		}
	}
}

void CSceneRenderPass::draw(std::vector <std::unique_ptr<IBatch> > & batches, IGPUBuffer& cameraData, IGPUBuffer& lightData)
{
	auto& device = COpenGLDevice::get();

	static const float vClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	device.glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);

	device.glViewport(0, 0, m_width, m_height);

	for (int i = 0; i < m_colorOutputs.size(); ++i)
	{
		device.glClearNamedFramebufferfv(m_framebufferObject, GL_COLOR, i, vClearColor);
	}

	if (m_depthOutput)
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

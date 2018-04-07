#include "renderpass.h"
#include "program.h"
#include "batch.h"
#include "resourcemanager.h"
#include "render/irenderpass.h"
#include "render/itexture.h"
#include "render/idevice.h"
#include "render/igpubuffer.h"
#include <iostream>

struct SFullScreenData
{
	SFullScreenData(IPipeline* pipeline, IDevice* device)
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
	IPipeline* m_pipeline;
};

CFullScreenRenderPass::CFullScreenRenderPass(IPipeline* pipeline, IDevice* device)
{
	m_data = std::make_unique <SFullScreenData> (pipeline, device);
	m_renderpass = device->createRenderPass();

	// create sampler for texture sampling of material
//	auto& gldevice = COpenGLDevice::get();
//	gldevice.glCreateSamplers(1, &m_sampler);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

CFullScreenRenderPass::~CFullScreenRenderPass()
{
//	auto& device = COpenGLDevice::get();

//	device.glDeleteSamplers(1, &m_sampler);
//	m_sampler = 0;
}

void CFullScreenRenderPass::setupRenderPass(ITexture** inputs, uint32_t numInputs, uint32_t width, uint32_t height)
{
	m_inputs.clear();
	m_inputs.reserve(numInputs);

	for (uint32_t i = 0; i < numInputs; ++i)
	{
		m_inputs.push_back(inputs[i]);
	}

	// temporary, these should be arguments in the function
	m_renderpass->setupRenderPass(nullptr, 0, nullptr);
}


void CFullScreenRenderPass::draw(ICommandBuffer& cmd)
{
	// test clear color
	static const float vClearColor[] = {1.0f, 0.0f, 0.0f, 0.0f};

	ICommandBuffer::CScopedRenderPass pass(cmd, *m_renderpass, vClearColor);

	cmd.bindPipeline(m_data->m_pipeline);
	cmd.setVertexStream(m_data->m_fullScreenTriangle.get());

//	for (uint32_t i = 0; i < m_inputs.size(); ++i)
//	{
//		m_inputs[i]->bind(i);
//		device.glBindSampler(i, m_sampler);
//	}

//	device.glDrawArrays(GL_TRIANGLES, 0, 3);
	cmd.drawArrays();
}

CSceneRenderPass::CSceneRenderPass(IDevice* device)
{
	m_renderpass = device->createRenderPass();
}

CSceneRenderPass::~CSceneRenderPass()
{
}

void CSceneRenderPass::draw(ICommandBuffer& cmd, std::vector <std::unique_ptr<IBatch> > & batches, IGPUBuffer& cameraData, IGPUBuffer& lightData)
{
	static const float vClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	static const float vClearDepth = 0.0;

	ICommandBuffer::CScopedRenderPass pass(cmd, *m_renderpass, vClearColor, &vClearDepth);
//	device.glEnable(GL_DEPTH_TEST);

//	device.glDepthFunc(GL_GEQUAL);
//	// inverse depth trick
//	device.glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
//	device.glDepthRangef(1.0f, 0.0f);

//	COpenGLBuffer& bglCameraData = static_cast<COpenGLBuffer&>(cameraData);
//	COpenGLBuffer& bglLightData = static_cast<COpenGLBuffer&>(lightData);

//	device.glBindBufferBase(GL_UNIFORM_BUFFER, 0, bglCameraData.getID());
//	device.glBindBufferBase(GL_UNIFORM_BUFFER, 1, bglLightData.getID());

	for (auto& batch : batches)
	{
		batch->draw(cmd);
	}

	//	device.glDisable(GL_DEPTH_TEST);
}

void CSceneRenderPass::setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	m_renderpass->setupRenderPass(outputs, numOutputs, depthOut);
}

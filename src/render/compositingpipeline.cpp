#include "compositingpipeline.h"
#include "batch.h"
#include "irenderpass.h"
#include "ipipeline.h"
#include "itexture.h"
#include "idevice.h"
#include "igpubuffer.h"
#include <iostream>

struct SFullScreenData
{
	SFullScreenData(IDevice* device)
	{
		m_fullScreenTriangle = device->createGPUBuffer(3 * sizeof(VertexFormatV), IGPUBuffer::Usage::eConstantVertex);

		if (auto lock = IGPUBuffer::CAutoLock <VertexFormatV>(*m_fullScreenTriangle))
		{
			lock[0].vertex = Vec3(-1.0, -1.0, 0.0);
			lock[1].vertex = Vec3(3.0, -1.0, 0.0);
			lock[2].vertex = Vec3(-1.0, 3.0, 0.0);
		}
	}

	std::unique_ptr <IGPUBuffer> m_fullScreenTriangle;
	std::unique_ptr <IPipeline>  m_pipeline;
};

IFullScreenRenderPass::IFullScreenRenderPass(IDevice* device)
{
	m_renderpass = device->createRenderPass();
	m_data = std::make_unique <SFullScreenData> (device);

	// create sampler for texture sampling of material
//	auto& gldevice = COpenGLDevice::get();
//	gldevice.glCreateSamplers(1, &m_sampler);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	gldevice.glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

IFullScreenRenderPass::~IFullScreenRenderPass()
{
//	auto& device = COpenGLDevice::get();

//	device.glDeleteSamplers(1, &m_sampler);
//	m_sampler = 0;
}

void IFullScreenRenderPass::setupRenderPass(IDevice& device, ITexture** inputs, uint32_t numInputs, ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	m_inputs.clear();
	m_inputs.reserve(numInputs);

	for (uint32_t i = 0; i < numInputs; ++i)
	{
		m_inputs.push_back(inputs[i]);
	}

	m_renderpass->setupRenderPass(outputs, numOutputs, depthOut);

	setupPipelines(device);
}


void IFullScreenRenderPass::draw(ICommandBuffer& cmd)
{
	// test clear color
	static const float vClearColor[] = {1.0f, 0.0f, 0.0f, 0.0f};

	ICommandBuffer::CScopedRenderPass pass(cmd, *m_renderpass, vClearColor);

	cmd.bindPipeline(m_data->m_pipeline.get());
	cmd.setVertexStream(m_data->m_fullScreenTriangle.get());

//	for (uint32_t i = 0; i < m_inputs.size(); ++i)
//	{
//		m_inputs[i]->bind(i);
//		device.glBindSampler(i, m_sampler);
//	}

	cmd.drawArrays(ICommandBuffer::EPrimitiveType::eTriangles, 0, 3);
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

//	COpenGLBuffer& bglCameraData = static_cast<COpenGLBuffer&>(cameraData);
//	COpenGLBuffer& bglLightData = static_cast<COpenGLBuffer&>(lightData);

//	device.glBindBufferBase(GL_UNIFORM_BUFFER, 0, bglCameraData.getID());
//	device.glBindBufferBase(GL_UNIFORM_BUFFER, 1, bglLightData.getID());

	for (auto& batch : batches)
	{
		cmd.bindPipeline(m_pipelines[batch->getPipeline()].get());
		batch->draw(cmd);
	}
}

void CSceneRenderPass::setupRenderPass(IDevice& device, ITexture** outputs, uint32_t numOutputs, ITexture* depthOut)
{
	m_renderpass->setupRenderPass(outputs, numOutputs, depthOut);
	setupPipelines(device);
}

void CSceneRenderPass::setupPipelines(IDevice& device)
{
	// check first pipeline for existence. if it exists, we have already run this code
	if (!m_pipelines[eDiffuse])
	{
		SSamplerParams sampler;

		SPipelineParams params;
		SDescriptorSet globalSet;

		params.addSampler(sampler);
		globalSet.addUniformBlock(eVertexStage);
		globalSet.addUniformBlock(eFragmentStage);

		params.globalSet = &globalSet;
		params.renderpass = m_renderpass.get();
		params.flags = eDepthCompareGreater | eCullBackFace;

		SVertexBinding instanceBinding(sizeof(MeshInstanceData));
		instanceBinding.addAttribute(0, eFloat, 4);
		instanceBinding.addAttribute(4 * sizeof(float), eFloat, 4);
		instanceBinding.addAttribute(8 * sizeof(float), eFloat, 4);
		instanceBinding.addAttribute(12 * sizeof(float), eFloat, 4);

		params.perInstanceBinding = &instanceBinding;

		{
			params.shaderModule = "generic";

			SVertexBinding vertBinding(sizeof(VertexFormatVN));
			vertBinding.addAttribute(offsetof(VertexFormatVN, vertex), eFloat, 3);
			vertBinding.addAttribute(offsetof(VertexFormatVN, normal), e1010102int, 4);

			params.perVertBinding = &vertBinding;
			m_pipelines[eDiffuse] = device.createPipeline(params);
		}

		{
			SDescriptorSet perDrawSet;
			perDrawSet.addTextureSlot(eFragmentStage, 0);

			params.shaderModule = "genericTextured";
			params.perDrawSet = &perDrawSet;

			SVertexBinding vertBinding(sizeof(VertexFormatVNT));
			vertBinding.addAttribute(offsetof(VertexFormatVNT, vertex), eFloat, 3);
			vertBinding.addAttribute(offsetof(VertexFormatVNT, normal), e1010102int, 4);
			vertBinding.addAttribute(offsetof(VertexFormatVNT, texCoord), eFloat, 2);
			params.perVertBinding = &vertBinding;

			m_pipelines[eDiffuseTextured] = device.createPipeline(params);

			params.flags |= ePrimitiveRestart;
			m_pipelines[eDiffuseTexturedPrimRestart] = device.createPipeline(params);
		}
	}
}

CToneMappingPass::~CToneMappingPass()
{

}

void CToneMappingPass::setupPipelines(IDevice& device)
{
	if (!m_data->m_pipeline)
	{
		SSamplerParams sampler;
		SDescriptorSet descriptorSet;

		SPipelineParams params;
		params.addSampler(sampler);
		descriptorSet.addTextureSlot(eFragmentStage, 0);
		params.globalSet = &descriptorSet;

		params.renderpass = m_renderpass.get();
		params.shaderModule = "toneMapping";
		SVertexBinding vertBinding(sizeof(VertexFormatV));
		vertBinding.addAttribute(offsetof(VertexFormatV, vertex), eFloat, 3);
		params.perVertBinding = &vertBinding;

		m_data->m_pipeline = device.createPipeline(params);
	}
}

CCompositingPipeline::CCompositingPipeline(IDevice* device)
	: m_sceneDrawPass(device)
	, m_toneMappingPass(device)
	, m_device(device)
{
}

CCompositingPipeline::~CCompositingPipeline()
{
}

void CCompositingPipeline::draw(ICommandBuffer& cmd, std::vector <std::unique_ptr<IBatch> > & batches, IGPUBuffer& cameraData, IGPUBuffer& lightData)
{
	m_sceneDrawPass.draw(cmd, batches, cameraData, lightData);
	m_toneMappingPass.draw(cmd);
}

void CCompositingPipeline::resize(IDevice& device, uint32_t width, uint32_t height)
{
	m_sceneHDRTex = m_device->createTexture(ITexture::eRGB16f, ITexture::EUsage::eSampled | ITexture::EUsage::eAttachement, width, height);
	m_DepthTex = m_device->createTexture(ITexture::eDepth32f, ITexture::EUsage::eAttachement, width, height);

	ITexture* sceneTex = m_sceneHDRTex.get();
	m_sceneDrawPass.setupRenderPass(device, &sceneTex, 1, m_DepthTex.get());
	m_toneMappingPass.setupRenderPass(device, &sceneTex, 1);
}

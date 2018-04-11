#include "compositingpipeline.h"
#include "batch.h"
#include "irenderpass.h"
#include "ipipeline.h"
#include "itexture.h"
#include "idevice.h"
#include "igpubuffer.h"
#include <iostream>
#include <algorithm>

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
	SRenderPassParams params;
	m_renderpass = device->createRenderPass(params);
	m_data = std::make_unique <SFullScreenData> (device);
}

IFullScreenRenderPass::~IFullScreenRenderPass()
{
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
	ICommandBuffer::CScopedRenderPass pass(cmd, *m_renderpass);

	cmd.bindPipeline(m_data->m_pipeline.get(), 1);

	std::vector <SDescriptorSource> descriptorSource;
	descriptorSource.reserve(m_inputs.size());
	for (auto& input : m_inputs)
	{
		descriptorSource.emplace_back(input);
	}

	cmd.bindGlobalPipelineDescriptors(descriptorSource.size(), descriptorSource.data());

	cmd.setVertexStream(m_data->m_fullScreenTriangle.get());

	cmd.drawArrays(ICommandBuffer::EPrimitiveType::eTriangles, 0, 3);
}

CSceneRenderPass::CSceneRenderPass(IDevice* device)
{
	SRenderPassParams params;
	SDescriptorSet globalSet;

	globalSet.addUniformBlock(eVertexStage);
	globalSet.addUniformBlock(eFragmentStage);
	params.set = &globalSet;

	m_renderpass = device->createRenderPass(params);
}

CSceneRenderPass::~CSceneRenderPass()
{
}

void CSceneRenderPass::draw(ICommandBuffer& cmd, std::vector <std::unique_ptr<IBatch> > & batches, IGPUBuffer& cameraData, IGPUBuffer& lightData)
{
	static const float vClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	static const float vClearDepth = 0.0;

	ICommandBuffer::CScopedRenderPass pass(cmd, *m_renderpass, vClearColor, &vClearDepth);

	std::array <SDescriptorSource, 2> descriptorSource = {
		SDescriptorSource{&cameraData},
		SDescriptorSource{&lightData}
	};

	cmd.bindGlobalRenderPassDescriptors(descriptorSource.size(), descriptorSource.data());

	auto predicate = [] (const std::unique_ptr<IBatch> &b1, const std::unique_ptr<IBatch> & b2) {
		return b1->getPipeline() < b2->getPipeline();
	};

	std::sort(batches.begin(), batches.end(), predicate);

	auto start = batches.begin();

	while(start != batches.end())
	{
		uint32_t pipelineNum = (*start)->getPipeline();
		IPipeline* pipeline = m_pipelines[pipelineNum].get();

		auto end = std::upper_bound(start, batches.end(), *start, predicate);
		cmd.bindPipeline(pipeline, end - start);

		while (start != end)
		{
			(*start++)->draw(cmd);
		}

		start = end;
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
		params.addSampler(sampler);
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

			params.perDrawBinding = &vertBinding;
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
			params.perDrawBinding = &vertBinding;

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
		sampler.bLinearFilter = false;
		sampler.bMipmapping = false;
		sampler.bRepeat = false;

		SDescriptorSet descriptorSet;

		SPipelineParams params;
		params.addSampler(sampler);
		descriptorSet.addTextureSlot(eFragmentStage, 0);
		params.globalSet = &descriptorSet;

		params.renderpass = m_renderpass.get();
		params.shaderModule = "toneMapping";
		SVertexBinding vertBinding(sizeof(VertexFormatV));
		vertBinding.addAttribute(offsetof(VertexFormatV, vertex), eFloat, 3);
		params.perDrawBinding = &vertBinding;

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

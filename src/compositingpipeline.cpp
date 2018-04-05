#include "compositingpipeline.h"
#include "render/idevice.h"
#include "resourcemanager.h"

CCompositingPipeline::CCompositingPipeline(ResourceManager* resourceManager, IDevice* device)
	: m_toneMappingPass(resourceManager->loadPipeline(eToneMapping), device)
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

void CCompositingPipeline::resize(uint32_t width, uint32_t height)
{
	m_sceneHDRTex = m_device->createTexture(ITexture::eRGB16f, width, height);
	m_DepthTex = m_device->createTexture(ITexture::eDepth32f, width, height);

	ITexture* sceneTex = m_sceneHDRTex.get();
	m_sceneDrawPass.setupRenderPass(&sceneTex, 1, m_DepthTex.get());
	m_toneMappingPass.setupRenderPass(&sceneTex, 1, width, height);
}

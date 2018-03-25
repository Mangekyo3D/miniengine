#include "compositingpipeline.h"
#include "texture.h"

CCompositingPipeline::CCompositingPipeline()
	: m_toneMappingPass("toneMapping")
{
}

CCompositingPipeline::~CCompositingPipeline()
{
}

void CCompositingPipeline::draw(std::vector <std::unique_ptr<IBatch> > & batches, IGPUBuffer& cameraData, IGPUBuffer& lightData)
{
	m_sceneDrawPass.draw(batches, cameraData, lightData);
	m_toneMappingPass.draw();
}

void CCompositingPipeline::resize(uint32_t width, uint32_t height)
{
	m_sceneHDRTex = std::make_unique <CTexture> (CTexture::eRGB16f, width, height);
	m_DepthTex = std::make_unique <CTexture> (CTexture::eDepth32f, width, height);

	CTexture* sceneTex = m_sceneHDRTex.get();
	m_sceneDrawPass.setupRenderPass(&sceneTex, 1, m_DepthTex.get());
	m_toneMappingPass.setupRenderPass(&sceneTex, 1, width, height);
}

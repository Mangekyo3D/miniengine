#include "compositingpipeline.h"
#include "texture.h"

CCompositingPipeline::CCompositingPipeline()
	: m_toneMappingPass("toneMapping")
{
}

CCompositingPipeline::~CCompositingPipeline()
{
}

void CCompositingPipeline::draw(std::vector <std::unique_ptr<IBatch> > & batches, uint32_t cameraID, uint32_t lightID)
{
	m_sceneDrawPass.draw(batches, cameraID, lightID);
	m_toneMappingPass.draw();
}

void CCompositingPipeline::resize(uint32_t width, uint32_t height)
{
	m_sceneDrawPass.resetOutputs();

	m_sceneHDRTex.reset(new CTexture(CTexture::eRGB16f, width, height));
	m_DepthTex.reset(new CTexture(CTexture::eDepth32f, width, height));

	m_sceneDrawPass.addColorOutput(m_sceneHDRTex.get());
	m_sceneDrawPass.addDepthOutput(m_DepthTex.get());
	m_sceneDrawPass.finalize();

	m_toneMappingPass.resetInputs();
	m_toneMappingPass.addInput(m_sceneHDRTex.get());
}

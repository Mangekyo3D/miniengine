#pragma once
#include "renderpass.h"
class CTexture;
class IBatch;

class CCompositingPipeline
{
	public:
		CCompositingPipeline();
		~CCompositingPipeline();

		void draw(std::vector<std::unique_ptr<IBatch> >& batches, uint32_t cameraID, uint32_t lightID);
		void resize(uint32_t width, uint32_t height);

	private:
		CSceneRenderPass m_sceneDrawPass;
		CFullScreenRenderPass m_toneMappingPass;

		std::unique_ptr <CTexture> m_sceneHDRTex;
		std::unique_ptr <CTexture> m_DepthTex;
};

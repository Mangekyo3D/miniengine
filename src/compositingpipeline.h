#pragma once
#include "renderpass.h"
class ITexture;
class IBatch;
class IGPUBuffer;
class IDevice;
class ResourceManager;
class ICommandBuffer;

class CCompositingPipeline
{
	public:
		CCompositingPipeline(ResourceManager* resourceManager, IDevice* device);
		~CCompositingPipeline();

		void draw(ICommandBuffer& cmd, std::vector<std::unique_ptr<IBatch> >& batches, IGPUBuffer& cameraData, IGPUBuffer& lightData);
		void resize(uint32_t width, uint32_t height);

	private:
		CSceneRenderPass m_sceneDrawPass;
		CFullScreenRenderPass m_toneMappingPass;

		std::unique_ptr <ITexture> m_sceneHDRTex;
		std::unique_ptr <ITexture> m_DepthTex;

		IDevice* m_device;
};

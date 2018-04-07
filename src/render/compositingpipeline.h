#pragma once
#pragma once
#include <vector>
#include <string>
#include <memory>


class ResourceManager;
class ITexture;
class IBatch;
class IGPUBuffer;
class IDevice;
class IPipeline;
class ICommandBuffer;
class IRenderPass;

struct SFullScreenData;

class CFullScreenRenderPass
{
	public:
		CFullScreenRenderPass(IPipeline* pipeline, IDevice* device);
		~CFullScreenRenderPass();
		// setup the renderpass with inputs and default framebuffer output.
		void setupRenderPass(ITexture** inputs, uint32_t numInputs, ITexture** outputs = nullptr, uint32_t numOutputs = 0, ITexture* depthOut = 0);
		void draw(ICommandBuffer&);

	private:
		std::unique_ptr <IRenderPass>     m_renderpass;
		std::unique_ptr <SFullScreenData> m_data;
		std::vector <ITexture*> m_inputs;

		uint32_t m_sampler;
};

class CSceneRenderPass
{
	public:
		CSceneRenderPass(IDevice* device);
		~CSceneRenderPass();

		void draw(ICommandBuffer&, std::vector<std::unique_ptr<IBatch> >& batches, IGPUBuffer& cameraData, IGPUBuffer& lightData);
		void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);

	private:
		std::unique_ptr <IRenderPass>     m_renderpass;
};

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

#pragma once
#include <vector>
#include <string>
#include <memory>

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
		void setupRenderPass(ITexture** inputs, uint32_t numInputs, uint32_t width, uint32_t height);

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

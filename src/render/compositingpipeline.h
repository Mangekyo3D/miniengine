#pragma once
#include "renderer.h"

#include <vector>
#include <string>
#include <memory>
#include <array>

class ITexture;
class IBatch;
class IGPUBuffer;
class IDevice;
class IPipeline;
class ICommandBuffer;
class IRenderPass;

struct SFullScreenData;

class IFullScreenRenderPass
{
	public:
		IFullScreenRenderPass(IDevice& device);
		virtual ~IFullScreenRenderPass();
		// setup the renderpass with inputs and default framebuffer output.
		void setupRenderPass(IDevice& device, ITexture** inputs, uint32_t numInputs, ITexture** outputs = nullptr, uint32_t numOutputs = 0, ITexture* depthOut = nullptr);
		void draw(ICommandBuffer&);

	protected:
		virtual void setupPipelines(IDevice& device) = 0;

		std::unique_ptr <IRenderPass>     m_renderpass;
		std::unique_ptr <SFullScreenData> m_data;
		std::vector <ITexture*> m_inputs;
};

class CToneMappingPass : public IFullScreenRenderPass
{
	public:
		CToneMappingPass(IDevice& device) : IFullScreenRenderPass(device) {}
		~CToneMappingPass() override;

	private:
		virtual void setupPipelines(IDevice& device) override;
};

class CSceneRenderPass
{
	public:
		CSceneRenderPass(IDevice& device);
		~CSceneRenderPass();

		void draw(ICommandBuffer&, std::vector<std::unique_ptr<IBatch> >& batches, IGPUBuffer& cameraData, IGPUBuffer& lightData);
		void setupRenderPass(IDevice& device, ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);

	private:
		void setupPipelines(IDevice& device);
		std::unique_ptr <IRenderPass>     m_renderpass;
		std::array <std::unique_ptr <IPipeline>, eMaxScenePipelines> m_pipelines;
};

class CCompositingPipeline
{
	public:
		CCompositingPipeline(IDevice& device, uint32_t width, uint32_t height);
		~CCompositingPipeline();

		void draw(ICommandBuffer& cmd, std::vector<std::unique_ptr<IBatch> >& batches, IGPUBuffer& cameraData, IGPUBuffer& lightData);
		void resize(IDevice& device, uint32_t width, uint32_t height);

	private:
		CSceneRenderPass m_sceneDrawPass;
		CToneMappingPass m_toneMappingPass;

		std::unique_ptr <ITexture> m_sceneHDRTex;
		std::unique_ptr <ITexture> m_DepthTex;

		IDevice* m_device;
};

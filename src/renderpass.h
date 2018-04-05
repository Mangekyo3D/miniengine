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

class CRenderPass
{
	public:
		CRenderPass();
		virtual ~CRenderPass();

		// setup the renderpass with inputs and outputs.
		void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);

	protected:
		uint32_t m_numOutputs;
		bool     m_bDepthOutput;

		// width and height, used when we render to default framebuffer
		uint32_t                m_width;
		uint32_t                m_height;

		// framebuffer object of this renderpass
		uint32_t m_framebufferObject;
};

struct SFullScreenData;

class CFullScreenRenderPass : public CRenderPass
{
	public:
		CFullScreenRenderPass(IPipeline* pipeline, IDevice* device);
		~CFullScreenRenderPass();
		// setup the renderpass with inputs and default framebuffer output.
		void setupRenderPass(ITexture** inputs, uint32_t numInputs, uint32_t width, uint32_t height);

		void draw(ICommandBuffer&);

	private:
		std::unique_ptr <SFullScreenData> m_data;
		std::vector <ITexture*> m_inputs;

		uint32_t m_sampler;
};

class CSceneRenderPass : public CRenderPass
{
	public:
		void draw(ICommandBuffer&, std::vector<std::unique_ptr<IBatch> >& batches, IGPUBuffer& cameraData, IGPUBuffer& lightData);
};

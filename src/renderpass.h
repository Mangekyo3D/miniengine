#pragma once
#include <vector>
#include <string>
#include <memory>

class CTexture;
class IBatch;
class IGPUBuffer;

class CRenderPass
{
	public:
		CRenderPass();
		virtual ~CRenderPass();

		// setup the renderpass with inputs and outputs.
		void setupRenderPass(CTexture** outputs, uint32_t numOutputs, CTexture* depthOut);

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
		CFullScreenRenderPass(std::string shaderName);
		~CFullScreenRenderPass();
		// setup the renderpass with inputs and default framebuffer output.
		void setupRenderPass(CTexture** inputs, uint32_t numInputs, uint32_t width, uint32_t height);

		void draw();

	private:
		std::unique_ptr <SFullScreenData> m_data;
		std::vector <CTexture*> m_inputs;

		uint32_t m_sampler;
};

class CSceneRenderPass : public CRenderPass
{
	public:
		void draw(std::vector<std::unique_ptr<IBatch> >& batches, IGPUBuffer& cameraData, IGPUBuffer& lightData);
};

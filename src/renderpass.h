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
		CRenderPass()
			: m_framebufferObject(0)
			, m_depthOutput(nullptr)
			, m_width(0)
			, m_height(0)
		{}
		virtual ~CRenderPass() {}

		void addColorOutput(CTexture* tex);
		void addDepthOutput(CTexture* tex);

		void setDefaultOutput(uint32_t width, uint32_t height)
		{
			m_width = width;
			m_height = height;
		}

		// deletes the framebuffer object and removes all outputs. Need to call finalize again after this
		// to reuse the renderpass
		void resetOutputs();

		// finalization either create a framebuffer object or not,
		// depending on whether a texture has been attached
		void finalize();

	protected:
		// outputs of the renderpass. Those are not owned by the renderpass,
		// rather they can be arbitrarily assigned by the pipeline that tracks
		// texture resources
		std::vector <CTexture*> m_colorOutputs;
		CTexture*               m_depthOutput;
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

		void addInput(CTexture* tex)
		{
			m_inputs.push_back(tex);
		}

		void resetInputs()
		{
			m_inputs.clear();
		}

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

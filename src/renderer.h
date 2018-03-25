#pragma once

#include <vector>
#include <memory>
#include "compositingpipeline.h"

class GameWindow;
class IDevice;
class Camera;
class IBatch;
class CCompositingPipeline;
class ResourceManager;

class Renderer
{
	public:
		Renderer(ResourceManager* resourceManager, std::unique_ptr<IDevice> device);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		void addNewBatch(std::unique_ptr<IBatch> batch);

		void updateFrameUniforms(Camera& camera);
		void drawFrame();

		void setViewport(uint32_t width, uint32_t height);

	private:
		/* batches that will be sent to GPU for rendering */
		std::vector <std::unique_ptr<IBatch> > m_batches;

		std::unique_ptr<IGPUBuffer> m_cameraUniform;
		std::unique_ptr<IGPUBuffer> m_lightUniform;

		std::unique_ptr <CCompositingPipeline> m_compositor;
		std::unique_ptr <IDevice> m_device;
};

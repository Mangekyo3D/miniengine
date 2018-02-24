#pragma once

#include <vector>
#include <memory>
#include "compositingpipeline.h"

class GameWindow;
class IDevice;
class Camera;
class IBatch;

class IRenderer
{
	public:
		virtual ~IRenderer() {}

		IRenderer(const IRenderer&) = delete;
		IRenderer& operator = (const IRenderer&) = delete;

		static IRenderer& get() { return *s_renderer.get(); }

		static void initialize(GameWindow&, bool bDebugContext, bool bVulkanContext);
		static void shutdown();

		virtual void addNewBatch(std::unique_ptr<IBatch> batch) = 0;

		virtual void updateFrameUniforms(Camera& camera) = 0;
		virtual void drawFrame() = 0;

		virtual void setViewport(uint32_t width, uint32_t height) = 0;

	protected:
		IRenderer() {}
		static std::unique_ptr <IRenderer> s_renderer;
		static std::unique_ptr <IDevice> s_device;
};

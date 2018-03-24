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
		IRenderer() {}
		virtual ~IRenderer() {}

		IRenderer(const IRenderer&) = delete;
		IRenderer& operator = (const IRenderer&) = delete;

		static std::unique_ptr<IRenderer> create(GameWindow&, bool bDebugContext, bool bVulkanContext);

		virtual void addNewBatch(std::unique_ptr<IBatch> batch) = 0;

		virtual void updateFrameUniforms(Camera& camera) = 0;
		virtual void drawFrame() = 0;

		virtual void setViewport(uint32_t width, uint32_t height) = 0;
};

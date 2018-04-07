#pragma once
#include "../irenderpass.h"

class CVulkanRenderPass : public IRenderPass
{
	public:
		CVulkanRenderPass();
		// setup the renderpass with inputs and outputs.
		virtual void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut);
};

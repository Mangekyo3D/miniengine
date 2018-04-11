#pragma once
#include <stdint.h>

class ITexture;
struct SDescriptorSet;

struct SRenderPassParams
{
	SDescriptorSet* set = nullptr;
};

class IRenderPass
{
	public:
		IRenderPass() {}
		virtual ~IRenderPass() {}

		// setup the renderpass with inputs and outputs.
		virtual void setupRenderPass(ITexture** outputs, uint32_t numOutputs, ITexture* depthOut) = 0;
};

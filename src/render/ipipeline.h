#pragma once
#include <stdint.h>
#include <vector>

class IRenderPass;

enum EPipelineFlags
{
	eDepthCompareGreater = 1L,
	eCullBackFace        = (1L << 1),
	ePrimitiveRestart    = (1L << 2)
};

enum EVertexFormat
{
	eFloat,
	e1010102int
};

enum EShaderStage {
	eVertexStage = 1,
	eFragmentStage = (1 << 1),
	eGeometryStage = (1 << 2),
	eTesselationEvalStage = (1 << 3),
	eTesselationControlStage = (1 << 4),
	eComputeStage = (1 << 5)
};

struct SVertexAttribParams
{
	SVertexAttribParams()
	{
	}

	SVertexAttribParams(uint32_t ofs, EVertexFormat fmt,uint8_t comp)
		: offset(ofs)
		, format(fmt)
		, components(comp)
	{
	}

	uint32_t offset;
	EVertexFormat format;
	uint8_t components;
};

struct SVertexBinding
{
	SVertexBinding(size_t ds)
		: dataSize(ds)
	{
	}

	void addAttribute(uint32_t offset, EVertexFormat format,uint8_t components)
	{
		attributeParams.emplace_back(offset, format,components);
	}

	std::vector <SVertexAttribParams> attributeParams;
	size_t                            dataSize;
};

struct SSamplerParams
{

};

enum EDescriptorType
{
	eUniformBlock,
	eTextureSampler
};

struct SDescriptorLayout
{
	SDescriptorLayout(){}

	SDescriptorLayout(EDescriptorType t, uint32_t stages, uint32_t smp = 0)
		: type(t)
		, shaderStages(stages)
		, sampler(smp)
	{
	}

	EDescriptorType type = eUniformBlock;
	uint32_t shaderStages = 0;
	uint32_t sampler = 0;
};

struct SPipelineParams
{
	IRenderPass* renderpass = nullptr;
	SVertexBinding* perVertBinding = nullptr;
	SVertexBinding* perInstanceBinding= nullptr;
	const char*     shaderModule = nullptr;
	uint64_t        flags = 0;

	void addUniformBlock(uint32_t stages)
	{
		descriptors.emplace_back(eUniformBlock, stages);
	}

	void addTextureSlot(uint32_t stages, uint32_t sampler)
	{
		descriptors.emplace_back(eTextureSampler, stages, sampler);
	}

	void addSampler(SSamplerParams& p)
	{
		samplers.push_back(p);
	}

	std::vector <SDescriptorLayout> descriptors;
	std::vector <SSamplerParams> samplers;
};

class IPipeline
{
	public:
		IPipeline() {}
		virtual ~IPipeline() {}

		virtual prepareDrawingResources() = 0;
};

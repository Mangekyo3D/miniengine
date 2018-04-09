#pragma once
#include <stdint.h>
#include <vector>


enum EPipelineFlags
{
	eDepthCompareGreater = 1L,
	eCullBackFace     = (1L << 1),
	ePrimitiveRestart = (1L << 2)
};

struct SPipelineParams
{
	SPipelineParams()
	{
		m_flags = 0;
	}

	uint64_t m_flags;
};

enum EVertexFormat
{
	eFloat,
	e1010102int
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
	SVertexBinding(size_t dataSize)
		: m_dataSize(dataSize)
	{
	}

	void addAttribute(uint32_t offset, EVertexFormat format,uint8_t components)
	{
		m_attributeParams.emplace_back(offset, format,components);
	}

	std::vector <SVertexAttribParams> m_attributeParams;
	size_t                            m_dataSize;
};

class IPipeline
{
	public:
		IPipeline() {}
		virtual ~IPipeline() {}
};

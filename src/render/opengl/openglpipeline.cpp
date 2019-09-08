#include "openglpipeline.h"
#include "opengldevice.h"
#include "openglbuffer.h"
#include <limits>

COpenGLVertexDescriptorInterface::COpenGLVertexDescriptorInterface(SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding)
	: m_perVertDataSize(0)
	, m_perInstanceDataSize(0)
{
	auto& device = COpenGLDevice::get();
	device.glCreateVertexArrays(1, &m_vertexArrayObject);

	uint32_t currentLocation = 0;

	if (perVertBinding)
	{
		m_perVertDataSize = perVertBinding->dataSize;
		for (const auto& attribute : perVertBinding->attributeParams)
		{
			device.glEnableVertexArrayAttrib(m_vertexArrayObject, currentLocation);
			device.glVertexArrayAttribBinding(m_vertexArrayObject, currentLocation, 0);
			device.glVertexArrayAttribFormat(m_vertexArrayObject, currentLocation, attribute.components, formatToGLFormat(attribute.format), GL_FALSE, attribute.offset);
			++currentLocation;
		}
	}

	if (perInstanceBinding)
	{
		m_perInstanceDataSize = perInstanceBinding->dataSize;
		device.glVertexArrayBindingDivisor(m_vertexArrayObject, 1, 1);
		for (const auto& attribute : perInstanceBinding->attributeParams)
		{
			device.glEnableVertexArrayAttrib(m_vertexArrayObject, currentLocation);
			device.glVertexArrayAttribBinding(m_vertexArrayObject, currentLocation, 1);
			device.glVertexArrayAttribFormat(m_vertexArrayObject, currentLocation, attribute.components, formatToGLFormat(attribute.format), GL_FALSE, attribute.offset);
			++currentLocation;
		}
	}
}

COpenGLVertexDescriptorInterface::~COpenGLVertexDescriptorInterface()
{
	auto& device = COpenGLDevice::get();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
}

void COpenGLVertexDescriptorInterface::setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf, IGPUBuffer* instanceBuf)
{
	auto& device = COpenGLDevice::get();

	GLuint vbID = 0;
	GLuint instID = 0;
	GLuint elemID = 0;

	if (vertexBuf && m_perVertDataSize > 0)
	{
		COpenGLBuffer* buf = static_cast<COpenGLBuffer*> (vertexBuf);
		vbID = buf->getID();
	}
	if (instanceBuf && m_perInstanceDataSize > 0)
	{
		COpenGLBuffer* buf = static_cast<COpenGLBuffer*> (instanceBuf);
		instID = buf->getID();
	}
	if (indexBuf)
	{
		COpenGLBuffer* buf = static_cast<COpenGLBuffer*> (indexBuf);
		elemID = buf->getID();
	}

	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vbID, 0, static_cast<GLsizei> (m_perVertDataSize));
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 1, instID, 0, static_cast<GLsizei> (m_perInstanceDataSize));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, elemID);
}

void COpenGLVertexDescriptorInterface::bind()
{
	auto& device = COpenGLDevice::get();
	device.glBindVertexArray(m_vertexArrayObject);
}

uint32_t COpenGLVertexDescriptorInterface::formatToGLFormat(EVertexFormat format)
{
	switch(format)
	{
		case eFloat:
			return  GL_FLOAT;
		case e1010102int:
			return GL_INT_2_10_10_10_REV;
	}

	return GL_FLOAT;
}

COpenGLPipeline::COpenGLPipeline(SPipelineParams& params, std::unique_ptr<COpenGLVertexDescriptorInterface> descriptor)
	: m_pipelineFlags(params.flags)
	, m_descriptor(std::move(descriptor))
	, m_samplers(params.samplers.size())
{
	COpenGLShader fragment_shader(params.shaderModule, EShaderStage::eFragmentStage);
	COpenGLShader vertex_shader(params.shaderModule, EShaderStage::eVertexStage);

	m_program.attach(vertex_shader);
	m_program.attach(fragment_shader);

	m_program.link();

	auto& device = COpenGLDevice::get();

	if (params.samplers.size() > 0)
	{
		device.glCreateSamplers(static_cast<GLsizei> (params.samplers.size()), m_samplers.data());
	}

	uint32_t samplerIndex = 0;
	for (auto& samplerParams : params.samplers)
	{
		uint32_t sampler = m_samplers[samplerIndex++];
		device.glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S,samplerParams.bRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		device.glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, samplerParams.bRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		device.glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, samplerParams.bLinearFilter ? GL_LINEAR : GL_NEAREST);
		device.glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, samplerParams.bMipmapping ?
									   (samplerParams.bLinearFilter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST):
									   (samplerParams.bLinearFilter ? GL_LINEAR : GL_NEAREST));
		device.glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, 0.0f);
		device.glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, samplerParams.bMipmapping ? std::numeric_limits <float>::max() : 0.25f);
		device.glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS, 0.0f);
	}

	uint32_t textureSlot = 0;
	if (params.globalSet)
	{
		for (auto& descriptor : params.globalSet->descriptors)
		{
			if (descriptor.type == eTextureSampler)
			{
				SamplerInfo samplerInfo = {m_samplers[descriptor.sampler], textureSlot++};
				m_samplerInfo.push_back(samplerInfo);
			}
		}
	}
	if (params.perDrawSet)
	{
		for (auto& descriptor : params.perDrawSet->descriptors)
		{
			if (descriptor.type == eTextureSampler)
			{
				SamplerInfo samplerInfo = {m_samplers[descriptor.sampler], textureSlot++};
				m_samplerInfo.push_back(samplerInfo);
			}
		}
	}
}

COpenGLPipeline::~COpenGLPipeline()
{
	auto& device = COpenGLDevice::get();
	device.glDeleteSamplers(static_cast<GLsizei> (m_samplers.size()), m_samplers.data());
}

COpenGLVertexDescriptorInterface* COpenGLPipeline::bind()
{
	// set up pipeline state for this pipeline
	auto& device = COpenGLDevice::get();

	// inverse depth trick. Some of these settings might be separated in the future
	if (m_pipelineFlags & eDepthCompareGreater)
	{
		device.glEnable(GL_DEPTH_TEST);
		device.glDepthFunc(GL_GEQUAL);
	}
	else
	{
		device.glDisable(GL_DEPTH_TEST);
		device.glDepthFunc(GL_ALWAYS);
	}

	if (m_pipelineFlags & eCullBackFace)
	{
		device.glEnable(GL_CULL_FACE);
		device.glCullFace(GL_BACK);
	}
	else
	{
		device.glDisable(GL_CULL_FACE);
	}

	if (m_pipelineFlags & ePrimitiveTypeTriangleStrip)
	{
		device.glEnable(GL_PRIMITIVE_RESTART);
		device.glPrimitiveRestartIndex(static_cast <uint16_t> (~0x0));
	}
	else
	{
		device.glDisable(GL_PRIMITIVE_RESTART);
	}

	m_program.use();
	m_descriptor->bind();

	for (auto& sampler : m_samplerInfo)
	{
		device.glBindSampler(static_cast<GLuint>(sampler.slot), sampler.sampler);
	}

	return m_descriptor.get();
}

bool COpenGLPipeline::getPrimitiveRestart()
{
    return (m_pipelineFlags & ePrimitiveRestart) != 0;
}

uint32_t COpenGLPipeline::getPrimitiveType()
{
    return ((m_pipelineFlags & ePrimitiveTypeTriangleStrip) != 0) ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
}

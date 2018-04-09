#pragma once
#include "../ipipeline.h"
#include "openglprogram.h"
#include <memory>
class IGPUBuffer;

// material descriptor wraps and sets up all data needed for a shader
class COpenGLVertexDescriptorInterface
{
	public:
		COpenGLVertexDescriptorInterface(SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding);
		~COpenGLVertexDescriptorInterface();
		void setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf = nullptr, IGPUBuffer* instanceBuf = nullptr);
		void bind();

	protected:
		uint32_t formatToGLFormat(EVertexFormat format);

		uint32_t m_vertexArrayObject;
		size_t   m_perVertDataSize;
		size_t   m_perInstanceDataSize;
};

class COpenGLPipeline : public IPipeline {
	public:
		COpenGLPipeline(SPipelineParams& params, std::string shaderFileName, std::unique_ptr<COpenGLVertexDescriptorInterface> descriptor);
		COpenGLVertexDescriptorInterface* bind();

	private:
		SPipelineParams m_params;
		COpenGLProgram m_program;
		std::unique_ptr <COpenGLVertexDescriptorInterface> m_descriptor;
};

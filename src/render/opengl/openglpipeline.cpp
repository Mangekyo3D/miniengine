#include "openglpipeline.h"
#include "opengldevice.h"
#include "openglbuffer.h"

COpenGLVertexDescriptorInterface::COpenGLVertexDescriptorInterface(SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding)
	: m_perVertDataSize(0)
	, m_perInstanceDataSize(0)
{
	auto& device = COpenGLDevice::get();
	device.glCreateVertexArrays(1, &m_vertexArrayObject);

	uint32_t currentLocation = 0;

	if (perVertBinding)
	{
		m_perVertDataSize = perVertBinding->m_dataSize;
		//device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vb->getID(), 0, perVertBinding->m_dataSize);
		for (const auto& attribute : perVertBinding->m_attributeParams)
		{
			device.glEnableVertexArrayAttrib(m_vertexArrayObject, currentLocation);
			device.glVertexArrayAttribBinding(m_vertexArrayObject, currentLocation, 0);
			device.glVertexArrayAttribFormat(m_vertexArrayObject, currentLocation, attribute.components, formatToGLFormat(attribute.format), GL_FALSE, attribute.offset);
			++currentLocation;
		}
	}

	if (perInstanceBinding)
	{
		m_perInstanceDataSize = perInstanceBinding->m_dataSize;
		for (const auto& attribute : perInstanceBinding->m_attributeParams)
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

COpenGLPipeline::COpenGLPipeline(std::string shaderFileName, std::unique_ptr<COpenGLVertexDescriptorInterface> descriptor)
	: m_descriptor(std::move(descriptor))
{
	CShader fragment_shader(shaderFileName, CShader::EType::eFragment);
	CShader vertex_shader(shaderFileName, CShader::EType::eVertex);

	m_program.attach(vertex_shader);
	m_program.attach(fragment_shader);

	m_program.link();
}

COpenGLVertexDescriptorInterface* COpenGLPipeline::bind()
{
	m_program.use();
	m_descriptor->bind();
	return m_descriptor.get();
}

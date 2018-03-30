#include "batch.h"
#include "render/opengl/opengldevice.h"
#include "render/itexture.h"
#include "render/opengl/openglbuffer.h"
#include <cstring>

COpenGLPipeline::COpenGLPipeline(std::string shaderFileName, std::unique_ptr <IDescriptorInterface> descriptor)
	: m_descriptor(std::move(descriptor))
{
	CShader fragment_shader(shaderFileName, CShader::EType::eFragment);
	CShader vertex_shader(shaderFileName, CShader::EType::eVertex);

	m_program.attach(vertex_shader);
	m_program.attach(fragment_shader);

	m_program.link();
}

IDescriptorInterface* COpenGLPipeline::bind()
{
	m_program.use();
	m_descriptor->bind();

	return m_descriptor.get();
}

void IDescriptorInterface::bind()
{
	auto& device = COpenGLDevice::get();
	device.glBindVertexArray(m_vertexArrayObject);
}

ArrayDescriptorV::ArrayDescriptorV()
{
	auto& device = COpenGLDevice::get();
	device.glCreateVertexArrays(1, &m_vertexArrayObject);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 0, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexFormatV, vertex));
}

ArrayDescriptorV::~ArrayDescriptorV()
{
	auto& device = COpenGLDevice::get();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
}

void ArrayDescriptorV::setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf, IGPUBuffer* instanceBuf)
{
	auto& device = COpenGLDevice::get();
	COpenGLBuffer* vb = static_cast<COpenGLBuffer*> (vertexBuf);

	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vb->getID(), 0, sizeof(VertexFormatV));
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 0);
}

IndexedInstancedDescriptorV::IndexedInstancedDescriptorV()
{
	auto& device = COpenGLDevice::get();
	device.glCreateVertexArrays(1, &m_vertexArrayObject);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 0, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexFormatVN, vertex));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 1, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 1, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexFormatVN, normal));

	// instance modelview matrix passed through vertex attributes and divisor
	int instanceMatrixLocation = 2;
	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation, 4, GL_FLOAT, GL_FALSE, 0);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation + 1, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation + 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation + 2, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation + 2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation + 3, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation + 3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float));

	device.glVertexArrayBindingDivisor(m_vertexArrayObject, 1, 1);
}

IndexedInstancedDescriptorV::~IndexedInstancedDescriptorV()
{
	auto& device = COpenGLDevice::get();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
}

void IndexedInstancedDescriptorV::setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf, IGPUBuffer* instanceBuf)
{
	auto& device = COpenGLDevice::get();
	COpenGLBuffer* vb = static_cast<COpenGLBuffer*> (vertexBuf);
	COpenGLBuffer* ib = static_cast<COpenGLBuffer*> (indexBuf);
	COpenGLBuffer* instb = static_cast<COpenGLBuffer*> (instanceBuf);

	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vb->getID(), 0, sizeof(VertexFormatVN));
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 1, instb->getID(), 0, sizeof(MeshInstanceData));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, ib->getID());

	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 0);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 1);

	int instanceMatrixLocation = 2;
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 1);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 2);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 3);
}

IndexedInstancedDescriptorVT::IndexedInstancedDescriptorVT()
{
	auto& device = COpenGLDevice::get();
	device.glCreateVertexArrays(1, &m_vertexArrayObject);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 0, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexFormatVNT, vertex));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 1, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 1, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexFormatVNT, normal));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 2, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 2, 2, GL_FLOAT, GL_FALSE, offsetof(VertexFormatVNT, texCoord));

	// instance modelview matrix passed through vertex attributes and divisor
	int instanceMatrixLocation = 3;
	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation, 4, GL_FLOAT, GL_FALSE, 0);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation + 1, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation + 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation + 2, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation + 2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, instanceMatrixLocation + 3, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, instanceMatrixLocation + 3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float));

	device.glVertexArrayBindingDivisor(m_vertexArrayObject, 1, 1);

	// create sampler for texture sampling of material
	device.glCreateSamplers(1, &m_sampler);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	device.glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

IndexedInstancedDescriptorVT::~IndexedInstancedDescriptorVT()
{
	auto& device = COpenGLDevice::get();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
	device.glDeleteSamplers(1, &m_sampler);
}

void IndexedInstancedDescriptorVT::setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf, IGPUBuffer* instanceBuf)
{
	auto& device = COpenGLDevice::get();
	COpenGLBuffer* vb = static_cast<COpenGLBuffer*> (vertexBuf);
	COpenGLBuffer* ib = static_cast<COpenGLBuffer*> (indexBuf);
	COpenGLBuffer* instb = static_cast<COpenGLBuffer*> (instanceBuf);

	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vb->getID(), 0, sizeof(VertexFormatVNT));
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 1, instb->getID(), 0, sizeof(MeshInstanceData));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, ib->getID());

	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 0);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 1);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 2);

	int instanceMatrixLocation = 3;
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 1);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 2);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 3);

	// bind sampler to duffuse texture unit
	device.glBindSampler(0, m_sampler);
}

CIndexedInstancedBatch::CIndexedInstancedBatch(IMesh *m, COpenGLPipeline *ma, const std::vector<ITexture *> *textures)
	: m_pipelineState(ma)
	, m_numInstances(0)
	, m_numIndices(m->getNumIndices())
	, m_bShortIndices(m->getIndexSize() == sizeof(uint16_t))
	, m_primType(m->m_primType)
	, m_bEnablePrimRestart(m->m_bEnablePrimRestart)
{
	if (textures)
	{
		 m_textures = *textures;
	}

	auto& device = COpenGLDevice::get();

	m_vertexBuffer = device.createGPUBuffer(m->getVertexSize() * m->getNumVertices());
	if (auto lock = IGPUBuffer::CAutoLock<uint8_t>(*m_vertexBuffer))
	{
		uint8_t* ptr = lock;
		memcpy(ptr, m->getVertices(), m->getVertexSize() * m->getNumVertices());
	}

	m_indexBuffer = device.createGPUBuffer(m->getIndexSize() * m->getNumIndices());
	if (auto lock = IGPUBuffer::CAutoLock<uint8_t>(*m_indexBuffer))
	{
		uint8_t* ptr = lock;
		memcpy(ptr, m->getIndices(), m->getIndexSize() * m->getNumIndices());
	}
}

CIndexedInstancedBatch::~CIndexedInstancedBatch()
{
}

static GLenum meshPrimitiveToGLPrimitive(IMesh::EPrimitiveType type)
{
	switch (type)
	{
		case IMesh::EPrimitiveType::eTriangles:
			return GL_TRIANGLES;
		case IMesh::EPrimitiveType::eTriangleStrip:
			return GL_TRIANGLE_STRIP;
		default:
			break;
	}

	return GL_TRIANGLES;
}

void CIndexedInstancedBatch::draw()
{
	if (m_instanceData.size() == 0)
	{
		return;
	}

	setupInstanceBuffer();

	IDescriptorInterface* desc = m_pipelineState->bind();
	desc->setVertexStream(m_vertexBuffer.get(), m_indexBuffer.get(), m_instanceBuffer.get());

	for (size_t i = 0, totalTex = m_textures.size(); i < totalTex; ++i)
	{
		m_textures[i]->bind(static_cast<uint8_t>(i));
	}

	auto& device = COpenGLDevice::get();
	device.glEnable(GL_CULL_FACE);

	if (m_bEnablePrimRestart)
	{
		device.glEnable(GL_PRIMITIVE_RESTART);
		device.glPrimitiveRestartIndex(0xFFFF);
	}

	device.glDrawElementsInstanced(meshPrimitiveToGLPrimitive(m_primType),static_cast <GLint> (m_numIndices),
								   (m_bShortIndices) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, nullptr,
								   static_cast <GLint> (m_instanceData.size()));

	device.glDisable(GL_CULL_FACE);
	if (m_bEnablePrimRestart)
	{
		device.glDisable(GL_PRIMITIVE_RESTART);
	}

	m_instanceData.clear();
}

void CIndexedInstancedBatch::addMeshInstance(MeshInstanceData& instance)
{
	m_instanceData.push_back(instance);
}

void CIndexedInstancedBatch::setupInstanceBuffer()
{
	auto& device = COpenGLDevice::get();

	// storage is immutable, so we have to reallocate
	if (m_instanceData.size() > m_numInstances)
	{
		m_instanceBuffer.reset();
	}

	if (!m_instanceBuffer)
	{
		m_instanceBuffer = device.createGPUBuffer(sizeof(MeshInstanceData) * m_instanceData.size());
		m_numInstances = static_cast <uint32_t> (m_instanceData.size());
	}

	if (auto lock = IGPUBuffer::CAutoLock<MeshInstanceData>(*m_instanceBuffer))
	{
		MeshInstanceData* ptr = lock;
		memcpy(ptr, m_instanceData.data(), sizeof(MeshInstanceData) * m_instanceData.size());
	}
}

CDynamicArrayBatch::CDynamicArrayBatch(COpenGLPipeline *material, const std::vector<ITexture *> *textures)
	: m_material(material)
{
	if (textures)
	{
		m_textures = *textures;
	}
}

CDynamicArrayBatch::~CDynamicArrayBatch()
{
}

void CDynamicArrayBatch::draw()
{

}

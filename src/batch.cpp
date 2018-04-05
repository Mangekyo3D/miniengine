#include "batch.h"
#include "render/opengl/opengldevice.h"
#include "render/itexture.h"
#include "render/opengl/openglbuffer.h"
#include <cstring>

CIndexedInstancedBatch::CIndexedInstancedBatch(IMesh *m, IPipeline* ma, const std::vector<ITexture *> *textures)
	: m_pipeline(ma)
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

void CIndexedInstancedBatch::draw(ICommandBuffer& cmd)
{
	if (m_instanceData.size() == 0)
	{
		return;
	}

	setupInstanceBuffer();

	cmd.bindPipeline(m_pipeline);
	cmd.setVertexStream(m_vertexBuffer.get(), m_indexBuffer.get(), m_instanceBuffer.get());

//	for (size_t i = 0, totalTex = m_textures.size(); i < totalTex; ++i)
//	{
//		m_textures[i]->bind(static_cast<uint8_t>(i));
//	}

//	auto& device = COpenGLDevice::get();
//	device.glEnable(GL_CULL_FACE);

//	if (m_bEnablePrimRestart)
//	{
//		device.glEnable(GL_PRIMITIVE_RESTART);
//		device.glPrimitiveRestartIndex(0xFFFF);
//	}

	cmd.drawIndexedInstanced();
//	device.glDrawElementsInstanced(meshPrimitiveToGLPrimitive(m_primType),static_cast <GLint> (m_numIndices),
//								   (m_bShortIndices) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, nullptr,
//								   static_cast <GLint> (m_instanceData.size()));

//	device.glDisable(GL_CULL_FACE);
//	if (m_bEnablePrimRestart)
//	{
//		device.glDisable(GL_PRIMITIVE_RESTART);
//	}

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

CDynamicArrayBatch::CDynamicArrayBatch(IPipeline* material, const std::vector<ITexture *> *textures)
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

void CDynamicArrayBatch::draw(ICommandBuffer&)
{

}

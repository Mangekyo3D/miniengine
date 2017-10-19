#include "batch.h"
#include "cdevice.h"
#include "texture.h"

Material::Material(std::string shaderFileName, std::unique_ptr <IMaterialDescriptor> descriptor)
	: m_descriptor(std::move(descriptor))
{
	Shader fragment_shader(shaderFileName + ".frag", Shader::EType::eFragment);
	Shader vertex_shader(shaderFileName + ".vert", Shader::EType::eVertex);

	m_program.attach(vertex_shader);
	m_program.attach(fragment_shader);

	m_program.link();
}

void Material::bind()
{
	auto device = IDevice::get <CDevice>();
	m_program.use();
}

GenericMaterialDescriptor::GenericMaterialDescriptor()
	: m_vertexArrayObject(0)
{
	auto device = IDevice::get <CDevice>();
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

GenericMaterialDescriptor::~GenericMaterialDescriptor()
{
	auto device = IDevice::get <CDevice>();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
}

void GenericMaterialDescriptor::setVertexStream(uint32_t vertexBuf, uint32_t indexBuf, uint32_t instanceBuf)
{
	auto device = IDevice::get <CDevice>();
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vertexBuf, 0, sizeof(VertexFormatVN));
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 1, instanceBuf, 0, sizeof(MeshInstanceData));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, indexBuf);

	// enable and bind the array
	device.glBindVertexArray(m_vertexArrayObject);

	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 0);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 1);

	int instanceMatrixLocation = 2;
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 1);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 2);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, instanceMatrixLocation + 3);
}

TexturedMaterialDescriptor::TexturedMaterialDescriptor()
	: m_vertexArrayObject(0)
{
	auto device = IDevice::get <CDevice>();
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

TexturedMaterialDescriptor::~TexturedMaterialDescriptor()
{
	auto device = IDevice::get <CDevice>();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
	device.glDeleteSamplers(1, &m_sampler);
}

void TexturedMaterialDescriptor::setVertexStream(uint32_t vertexBuf, uint32_t indexBuf, uint32_t instanceBuf)
{
	auto device = IDevice::get <CDevice>();
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vertexBuf, 0, sizeof(VertexFormatVNT));
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 1, instanceBuf, 0, sizeof(MeshInstanceData));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, indexBuf);

	device.glBindVertexArray(m_vertexArrayObject);

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

CBatch::CBatch(IMesh *m, Material *ma, const std::vector<CTexture *> *textures)
	: m_material(ma)
	, m_instanceBuffer(0)
	, m_numInstances(0)
	, m_numIndices(m->getNumIndices())
	, m_primType(m->m_primType)
	, m_bEnablePrimRestart(m->m_bEnablePrimRestart)
{
	if (textures)
	{
		 m_textures = *textures;
	}

	auto device = IDevice::get <CDevice>();

	device.glCreateBuffers(1, &m_vertexBuffer);
	device.glNamedBufferStorage(m_vertexBuffer, m->getVertexSize() * m->getNumVertices(), m->getVertices(), 0);

	device.glCreateBuffers(1, &m_indexBuffer);
	device.glNamedBufferStorage(m_indexBuffer, m->getIndexSize() * m->getNumIndices(), m->getIndices(), 0);
}

CBatch::~CBatch()
{
	auto device = IDevice::get <CDevice>();

	device.glDeleteBuffers(1, &m_vertexBuffer);
	device.glDeleteBuffers(1, &m_indexBuffer);
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

void CBatch::draw(uint32_t cameraUniformID, uint32_t lightUniformID)
{
	if (m_instanceData.size() == 0)
	{
		return;
	}

	setupInstanceBuffer();

	m_material->bind();

	IMaterialDescriptor& desc = m_material->getDescriptor();
	desc.setVertexStream(m_vertexBuffer, m_indexBuffer, m_instanceBuffer);

	for (size_t i = 0, totalTex = m_textures.size(); i < totalTex; ++i)
	{
		m_textures[i]->bind(static_cast<uint8_t>(i));
	}

	auto device = IDevice::get <CDevice>();
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUniformID);
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUniformID);
	device.glEnable(GL_CULL_FACE);

	if (m_bEnablePrimRestart)
	{
		device.glEnable(GL_PRIMITIVE_RESTART);
		device.glPrimitiveRestartIndex(0xFFFF);
	}

	device.glDrawElementsInstanced(meshPrimitiveToGLPrimitive(m_primType),static_cast <GLint> (m_numIndices),
								   GL_UNSIGNED_SHORT, nullptr, static_cast <GLint> (m_instanceData.size()));

	device.glDisable(GL_CULL_FACE);
	if (m_bEnablePrimRestart)
	{
		device.glDisable(GL_PRIMITIVE_RESTART);
	}

	m_instanceData.clear();
}

void CBatch::addMeshInstance(MeshInstanceData& instance)
{
	m_instanceData.push_back(instance);
}

void CBatch::setupInstanceBuffer()
{
	auto device = IDevice::get <CDevice>();

	// storage is immutable, so we have to reallocate
	if (m_instanceData.size() > m_numInstances)
	{
		device.glDeleteBuffers(1, &m_instanceBuffer);
		m_instanceBuffer = 0;
	}

	if (m_instanceBuffer == 0)
	{
		device.glCreateBuffers(1, &m_instanceBuffer);
		device.glNamedBufferStorage(m_instanceBuffer, sizeof(MeshInstanceData) * m_instanceData.size(), m_instanceData.data(), GL_DYNAMIC_STORAGE_BIT);
		m_numInstances = static_cast <uint32_t> (m_instanceData.size());
	}
	else
	{
		// invalidate first so that we avoid locking here
		device.glInvalidateBufferSubData(m_instanceBuffer, 0, sizeof(MeshInstanceData) * m_instanceData.size());
		device.glNamedBufferSubData(m_instanceBuffer, 0, sizeof(MeshInstanceData) * m_instanceData.size(), m_instanceData.data());
	}
}

CDynamicBatch::CDynamicBatch(Material *material, const std::vector<CTexture *> *textures)
	: m_material(material)
{
	if (textures)
	{
		m_textures = *textures;
	}
}

CDynamicBatch::~CDynamicBatch()
{
	auto device = IDevice::get <CDevice>();
	device.glDeleteBuffers(1, &m_vertexBuffer);
}

void CDynamicBatch::draw(uint32_t cameraUniformID, uint32_t lightUniformID)
{

}
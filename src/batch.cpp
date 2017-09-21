#include "batch.h"
#include "cdevice.h"

Material::Material(std::string name)
		: m_vertexArrayObject(0)
{
	Shader fragment_shader(name + ".frag", Shader::EType::eFragment);
	Shader vertex_shader(name + ".vert", Shader::EType::eVertex);

	m_program.attach(vertex_shader);
	m_program.attach(fragment_shader);

	m_program.link();

	auto device = IDevice::get <CDevice>();
	device.glCreateVertexArrays(1, &m_vertexArrayObject);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 0, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexFormatVN, vertex));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 1, 0);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 1, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexFormatVN, normal));

	// modelview matrix passed through vertex attributes
	device.glVertexArrayAttribBinding(m_vertexArrayObject, 2, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 2, 4, GL_FLOAT, GL_FALSE, 0);

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 3, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 4, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 4, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float));

	device.glVertexArrayAttribBinding(m_vertexArrayObject, 5, 1);
	device.glVertexArrayAttribFormat(m_vertexArrayObject, 5, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float));

	device.glVertexArrayBindingDivisor(m_vertexArrayObject, 1, 1);
}

Material::~Material()
{
	auto device = IDevice::get <CDevice>();
	device.glDeleteVertexArrays(1, &m_vertexArrayObject);
}

void Material::bind()
{
	auto device = IDevice::get <CDevice>();

	m_program.use();
	device.glBindVertexArray(m_vertexArrayObject);

	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 0);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 1);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 2);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 3);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 4);
	device.glEnableVertexArrayAttrib(m_vertexArrayObject, 5);
}

void Material::setVertexStream(uint32_t vertexBuf, uint32_t indexBuf, uint32_t instanceBuf)
{
	auto device = IDevice::get <CDevice>();
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vertexBuf, 0, sizeof(VertexFormatVN));
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 1, instanceBuf, 0, sizeof(MeshInstanceData));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, indexBuf);
}

Mesh::Mesh(uint32_t nov, uint32_t noi)
	: m_vertices(nov)
	, m_indices(noi)
	, m_primType(EPrimitiveType::eTriangles)
	, m_bEnablePrimRestart(false)
{
}


Mesh::~Mesh()
{
}


CBatch::CBatch(Mesh *m, Material *ma)
	: m_mesh(m)
	, m_material(ma)
	, m_instanceBuffer(0)
{
	auto device = IDevice::get <CDevice>();

	device.glCreateBuffers(1, &m_vertexBuffer);
	device.glNamedBufferStorage(m_vertexBuffer, sizeof(VertexFormatVN) * m_mesh->m_vertices.size(), m_mesh->m_vertices.data(), 0);

	device.glCreateBuffers(1, &m_indexBuffer);
	device.glNamedBufferStorage(m_indexBuffer, sizeof(uint16_t) * m_mesh->m_indices.size(), m_mesh->m_indices.data(), 0);

	m_numInstances = 0;
}

CBatch::~CBatch()
{
	auto device = IDevice::get <CDevice>();

	device.glDeleteBuffers(1, &m_vertexBuffer);
	device.glDeleteBuffers(1, &m_indexBuffer);
}

static GLenum meshPrimitiveToGLPrimitive(Mesh::EPrimitiveType type)
{
	switch (type)
	{
		case Mesh::EPrimitiveType::eTriangles:
			return GL_TRIANGLES;
		case Mesh::EPrimitiveType::eTriangleStrip:
			return GL_TRIANGLE_STRIP;
		default:
			break;
	}

	return GL_TRIANGLES;
}

void CBatch::draw(uint32_t cameraUniformID, uint32_t lightUniformID)
{
	setupInstanceBuffer();

	m_material->bind();
	m_material->setVertexStream(m_vertexBuffer, m_indexBuffer, m_instanceBuffer);

	auto device = IDevice::get <CDevice>();
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUniformID);
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUniformID);

	if (m_mesh->m_bEnablePrimRestart)
	{
		device.glEnable(GL_PRIMITIVE_RESTART);
		device.glPrimitiveRestartIndex(0xFFFF);
	}

	device.glDrawElementsInstanced(meshPrimitiveToGLPrimitive(m_mesh->m_primType),static_cast <GLint> (m_mesh->m_indices.size()),
								   GL_UNSIGNED_SHORT, nullptr, static_cast <GLint> (m_instanceData.size()));

	if (m_mesh->m_bEnablePrimRestart)
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

	if (m_instanceData.size() > m_numInstances)
	{
		device.glDeleteBuffers(1, &m_instanceBuffer);
		m_instanceBuffer = 0;
	}

	if (m_instanceBuffer == 0)
	{
		device.glCreateBuffers(1, &m_instanceBuffer);
		device.glNamedBufferStorage(m_instanceBuffer, sizeof(MeshInstanceData) * m_instanceData.size(), m_instanceData.data(), 0);
	}
	else
	{
		device.glNamedBufferSubData(m_instanceBuffer, 0, sizeof(MeshInstanceData) * m_instanceData.size(), m_instanceData.data());
	}
}

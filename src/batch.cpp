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
}

void Material::setVertexStream(uint32_t vertexBuf, uint32_t indexBuf)
{
	auto device = IDevice::get <CDevice>();
	device.glVertexArrayVertexBuffer(m_vertexArrayObject, 0, vertexBuf, 0, sizeof(VertexFormatVN));
	device.glVertexArrayElementBuffer(m_vertexArrayObject, indexBuf);
}

Mesh::Mesh(uint32_t nov, uint32_t noi)
	: m_vertices(nov)
	, m_indices(noi)
	, m_bEnablePrimRestart(false)
{
}


Mesh::~Mesh()
{
}


CBatch::CBatch(Mesh *m, Material *ma)
	: m_mesh(m)
	, m_material(ma)
{
	auto device = IDevice::get <CDevice>();

	device.glCreateBuffers(1, &m_vertexBuffer);
	device.glNamedBufferStorage(m_vertexBuffer, sizeof(VertexFormatVN) * m_mesh->m_vertices.size(), m_mesh->m_vertices.data(), 0);

	device.glCreateBuffers(1, &m_indexBuffer);
	device.glNamedBufferStorage(m_indexBuffer, sizeof(uint16_t) * m_mesh->m_indices.size(), m_mesh->m_indices.data(), 0);
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
	m_material->bind();
	m_material->setVertexStream(m_vertexBuffer, m_indexBuffer);

	auto device = IDevice::get <CDevice>();
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUniformID);
	device.glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUniformID);

	if (m_mesh->m_bEnablePrimRestart)
	{
		device.glEnable(GL_PRIMITIVE_RESTART);
		device.glPrimitiveRestartIndex(0xFFFF);
	}

	device.glDrawRangeElements(meshPrimitiveToGLPrimitive(m_mesh->m_primType), 0,
							   static_cast <GLint> (m_mesh->m_vertices.size()),
							   static_cast <GLint> (m_mesh->m_indices.size()), GL_UNSIGNED_SHORT, nullptr);

	if (m_mesh->m_bEnablePrimRestart)
	{
		device.glDisable(GL_PRIMITIVE_RESTART);
	}
}

void CBatch::addMeshInstance(MeshInstanceData& instance)
{
	m_instance_data.push_back(instance);
}

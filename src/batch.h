#pragma once
#include <vector>
#include <string>
#include "Util/vertex.h"
#include "program.h"

class Material {
	public:
		Material(std::string name);
		~Material();

		void bind();
		void setVertexStream(uint32_t vertexBuf, uint32_t indexBuf);

	private:
		Program m_program;

		uint32_t m_vertexArrayObject;
};

/* Mesh - one material per vertex buffer */
struct VertexFormatVN
{
	Vec3 vertex;
	uint32_t normal;
};

struct Mesh {
		enum class EPrimitiveType
		{
			eTriangles,
			eTriangleStrip
		};

		Mesh(uint32_t, uint32_t);
		~Mesh();

		std::vector <VertexFormatVN> m_vertices;
		std::vector <uint16_t> m_indices;

		EPrimitiveType m_primType;
		bool m_bEnablePrimRestart;
};


class MeshInstanceData {
	private:
		float modelMatrix[12];
};

class CBatch {
	public:
		CBatch(Mesh *, Material *);
		~CBatch();
		void draw(uint32_t cameraUniformID, uint32_t lightUniformID);
		void addMeshInstance(MeshInstanceData& instance);

		std::vector <MeshInstanceData> m_instance_data;

	private:
		Mesh* m_mesh;
		Material* m_material;
		uint32_t m_vertexBuffer;
		uint32_t m_indexBuffer;
};

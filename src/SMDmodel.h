#pragma once
#include "Util/vertex.h"
#include <vector>
#include <memory>

class CIndexedInstancedBatch;
class CTexture;
struct IMesh;

class SMDModel
{
	public:
		SMDModel() {}
		~SMDModel();
		bool openFromFile(const char *);
		std::unique_ptr <CIndexedInstancedBatch> createBatch();

	private:
		bool prepareVertexBuffer();

		std::vector <Vec3>     m_vertexData;
		std::vector <Vec3>     m_normalData;
		std::vector <Vec2>     m_texCoordData;
		std::vector <uint16_t> m_indexData;

		bool m_bUseTexture;

		std::vector <CTexture*> m_textures;
		std::unique_ptr <IMesh> m_mesh;
};

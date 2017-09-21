#pragma once
#include "Util/vertex.h"
#include <vector>
#include <memory>

class CBatch;
struct Mesh;

class SMDModel
{
	public:
		SMDModel() {}
		~SMDModel();
		bool openFromFile(const char *);
		CBatch* getBatch();

	private:
		bool prepareVertexBuffer();

		std::vector <Vec3>     m_vertexData;
		std::vector <Vec3>     m_normalData;
		std::vector <Vec2>     m_texCoords;
		std::vector <uint16_t> m_indexData;

		bool m_bUseTexture;

		std::unique_ptr <Mesh> m_mesh;
		CBatch* m_batch = nullptr;
};

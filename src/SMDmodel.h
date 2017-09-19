#pragma once
#include "Util/vertex.h"
#include <vector>
#include <memory>
#include "batch.h"

class SMDModel
{
	public:
		SMDModel() {}
		~SMDModel();
		bool openFromFile(const char *);
		bool prepareVertexBuffer();

	private:
		std::vector <Vec3>     m_vertexData;
		std::vector <Vec3>     m_normalData;
		std::vector <Vec2>     m_texCoords;
		std::vector <uint16_t> m_indexData;

		bool m_bUseTexture;

		unsigned int m_imageId;
		unsigned int m_samplerId;

		int floatsPerVertex;

		std::unique_ptr <Mesh> m_mesh;
		std::unique_ptr <Material> m_material;
};

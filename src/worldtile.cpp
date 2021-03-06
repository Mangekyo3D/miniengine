#include "worldtile.h"
#include "Util/perlin.h"
#include "Util/vertex.h"
#include "Util/matrix.h"
#include "render/renderer.h"
#include "render/compositingpipeline.h"
#include "resourcemanager.h"

class WorldTile::MeshAdapter : public IMeshAdapter
{
public:
	MeshAdapter(const WorldTile& parent) : m_parent(parent) {}

	size_t getVertexSize() override { return sizeof(VertexFormatVNT); }
	size_t getNumVertices() override { return m_parent.m_vertices.size(); }
	size_t getNumIndices() override { return m_parent.m_indices.size(); };
	size_t getIndexSize() override { return sizeof(m_parent.m_indices[0]); }
	void fillVertices(uint8_t* buffer) {
		memcpy(buffer, m_parent.m_vertices.data(), getNumVertices() * getVertexSize());
	}

	void fillIndices(uint8_t* buffer) override {
		memcpy(buffer, m_parent.m_indices.data(), getNumIndices() * getIndexSize());
	}

private:
	const WorldTile& m_parent;
};

WorldTile::WorldTile(uint16_t resolution)
	: m_resolution(resolution)
	, m_vertices(resolution * resolution)
	, m_indices((resolution-1) * (2 * resolution + 1))
{
	generateProcedural();
}

WorldTile::~WorldTile()
{
}


void WorldTile::setup_draw_operations(Renderer* renderer, ResourceManager* resourceManager)
{
	if (!m_batch)
	{
		std::vector <ITexture*> textures;
		textures.push_back(resourceManager->loadTexture("grass.bmp"));
		MeshAdapter adapter(*this);
		m_batch = renderer->addNewBatch<CIndexedInstancedBatch>(adapter, CSceneRenderPass::eTerrainPipeline, &textures);
	}

	MeshInstanceData data;
	// identity matrix by default
	Matrix44 modelMatrix;
	modelMatrix.copyData(data.modelMatrix);

	m_batch->addMeshInstance(data);

}

void WorldTile::generateProcedural()
{
	for (uint16_t i = 0; i < m_resolution; ++i)
	{
		for (uint16_t j = 0; j < m_resolution; ++j)
		{
			uint16_t index = i * m_resolution + j;

			float fHeight = 0.0f;
			uint32_t divisor = m_resolution / 2;

			while (divisor > 0)
			{
				fHeight += 0.25f * divisor * static_cast <float> (Perlin::noise(static_cast <double> (i) / divisor, static_cast <double> (j) / divisor, 0));
				divisor >>= 1;
			}

			m_vertices[index].vertex = Vec3(static_cast <float> (i), static_cast <float> (j), fHeight);
			m_vertices[index].texCoord = Vec2(static_cast <float> (i), static_cast <float> (j));
		}
	}

	// initialize the sides of the grid first
	for (uint16_t i = 0; i < m_resolution; ++i)
	{
		Vec3 normal(0.0f, 0.0f, 1.0f);
		unsigned int index = i * m_resolution;
		m_vertices[index].normal = normal;

		index = i * m_resolution + m_resolution - 1;
		m_vertices[index].normal = normal;

		m_vertices[i].normal = normal;
		m_vertices[i + m_resolution * (m_resolution - 1)].normal = normal;
	}

	for (uint16_t i = 1; i < m_resolution-1; ++i)
	{
		for (uint16_t j = 1; j < m_resolution-1; ++j)
		{
			uint32_t index = i * m_resolution + j;
			Vec3 normal;

			Vec3 c = m_vertices[i * m_resolution + j].vertex;
			Vec3 t1 = m_vertices[(i - 1) * m_resolution + j - 1].vertex;
			Vec3 t2 = m_vertices[i * m_resolution + j - 1].vertex;
			Vec3 t3 = m_vertices[(i + 1) * m_resolution + j - 1].vertex;
			Vec3 t4 = m_vertices[(i + 1) * m_resolution + j].vertex;
			Vec3 t5 = m_vertices[(i + 1) * m_resolution + j + 1].vertex;
			Vec3 t6 = m_vertices[i * m_resolution + j+1].vertex;
			Vec3 t7 = m_vertices[(i - 1) * m_resolution + j + 1].vertex;
			Vec3 t8 = m_vertices[(i - 1) * m_resolution + j].vertex;

			t1 -= c;
			t2 -= c;
			t3 -= c;
			t4 -= c;
			t5 -= c;
			t6 -= c;
			t7 -= c;
			t8 -= c;

			normal = cross(t1, t2) + cross(t2, t3) + cross(t3, t4) + cross(t4, t5)
			 + cross(t5, t6) + cross(t6, t7) + cross(t7, t8) + cross(t8, t1);

			normal.normalize();
			m_vertices[index].normal = normal;
		}
	}

	uint16_t index_iter = 0;

	for (uint16_t i = 0; i < m_resolution - 1; ++i)
	{
		m_indices[index_iter++] = i * m_resolution;
		m_indices[index_iter++] = (i + 1) * m_resolution;

		for (uint16_t j = 0; j < m_resolution - 1; ++j)
		{
			m_indices[index_iter++] = i * m_resolution + j + 1;
			m_indices[index_iter++] = (i + 1) * m_resolution + j + 1;
		}

		m_indices[index_iter++] = static_cast<uint16_t>(~0x0);
	}
}

float WorldTile::getHeightAt(float x, float y)
{
	uint16_t gx = static_cast<uint16_t > (x);
	uint16_t gy = static_cast<uint16_t > (y);

	float dx = x - gx;
	float dy = y - gy;

	if (gx >= m_resolution)
	{
		gx = m_resolution - 2;
		dx = 1.0f;
	}
	else if (x < 0.0f)
	{
		gx = 0;
		dx = 0;
	}

	if (gy >= m_resolution)
	{
		gy = m_resolution - 2;
		dy = 1.0f;
	}
	else if (y < 0.0f)
	{
		gy = 0;
		dy = 0;
	}

	if(dy + dx < 1)
	{
		float xt = getHeightAtGrid(gx +1, gy) - getHeightAtGrid(gx, gy);
		float yt = getHeightAtGrid(gx, gy + 1) - getHeightAtGrid(gx, gy);
		return getHeightAtGrid(gx, gy) + (dx*xt + dy*yt) + 0.2f;
	}
	else
	{
		dy = 1.0f - dy;
		dx = 1.0f - dx;
		float xt = getHeightAtGrid(gx + 1, gy + 1) - getHeightAtGrid(gx, gy + 1);
		float yt = getHeightAtGrid(gx + 1, gy + 1) - getHeightAtGrid(gx + 1, gy);
		return getHeightAtGrid(gx + 1, gy + 1) - (dx*xt + dy*yt) + 0.2f;
	}
}

float WorldTile::getHeightAtGrid(uint16_t x, uint16_t y)
{
	uint16_t index =  x * m_resolution + y;

	return m_vertices[index].vertex.z();
}

#include "worldtile.h"
#include "Util/perlin.h"
#include "Util/vertex.h"
#include "Util/matrix.h"
#include "renderer.h"
#include "resourcemanager.h"

WorldTile::WorldTile(uint16_t resolution)
	: m_resolution(resolution)
	, m_mesh(resolution * resolution, (resolution-1) * (2 * resolution + 1))
{
	generateProcedural();
}

WorldTile::~WorldTile()
{
}


void WorldTile::setup_draw_operations()
{
	Renderer& renderer = Renderer::get();

	if (!m_batch)
	{
		std::vector <CTexture*> textures;
		textures.push_back(ResourceManager::get().loadTexture("grass.bmp"));
		Material* material = ResourceManager::get().loadMaterial("genericTextured");

		auto newBatch = std::make_unique <CBatch> (&m_mesh, material, &textures);
		m_batch = newBatch.get();
		renderer.addNewBatch(std::move(newBatch));
	}

	MeshInstanceData data;
	// identity matrix by default
	Matrix44 modelMatrix;
	modelMatrix.getData(data.modelMatrix);

	m_batch->addMeshInstance(data);

}

void WorldTile::generateProcedural()
{
	m_mesh.m_primType = IMesh::EPrimitiveType::eTriangleStrip;
	m_mesh.m_bEnablePrimRestart = true;

	for (int i = 0; i < m_resolution; ++i)
	{
		for (int j = 0; j < m_resolution; ++j)
		{
			unsigned int index = i * m_resolution + j;

			float fHeight = 0.0f;
			uint16_t divisor = m_resolution / 2;

			while (divisor > 0)
			{
				fHeight += static_cast <float> (0.25f * divisor * Perlin::noise(static_cast <float> (i) / divisor, static_cast <float> (j) / divisor, 0));
				divisor >>= 1;
			}

			m_mesh.m_vertices[index].vertex = Vec3(static_cast <float> (i), static_cast <float> (j), fHeight);
			m_mesh.m_vertices[index].texCoord = Vec2(static_cast <float> (i), static_cast <float> (j));
		}
	}

	// initialize the sides of the grid first
	for (int i = 0; i < m_resolution; ++i)
	{
		Vec3 normal(0.0f, 0.0f, 1.0f);
		unsigned int index = i * m_resolution;
		m_mesh.m_vertices[index].normal = normal;

		index = i * m_resolution + m_resolution - 1;
		m_mesh.m_vertices[index].normal = normal;

		m_mesh.m_vertices[i].normal = normal;
		m_mesh.m_vertices[i + m_resolution * (m_resolution - 1)].normal = normal;
	}

	for (int i = 1; i < m_resolution-1; ++i)
	{
		for (int j = 1; j < m_resolution-1; ++j)
		{
			unsigned int index = i * m_resolution + j;
			Vec3 normal;

			Vec3 c = m_mesh.m_vertices[i * m_resolution + j].vertex;
			Vec3 t1 = m_mesh.m_vertices[(i - 1) * m_resolution + j - 1].vertex;
			Vec3 t2 = m_mesh.m_vertices[i * m_resolution + j - 1].vertex;
			Vec3 t3 = m_mesh.m_vertices[(i + 1) * m_resolution + j - 1].vertex;
			Vec3 t4 = m_mesh.m_vertices[(i + 1) * m_resolution + j].vertex;
			Vec3 t5 = m_mesh.m_vertices[(i + 1) * m_resolution + j + 1].vertex;
			Vec3 t6 = m_mesh.m_vertices[i * m_resolution + j+1].vertex;
			Vec3 t7 = m_mesh.m_vertices[(i - 1) * m_resolution + j + 1].vertex;
			Vec3 t8 = m_mesh.m_vertices[(i - 1) * m_resolution + j].vertex;

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
			m_mesh.m_vertices[index].normal = normal;
		}
	}

	int index_iter = 0;

	for (int i = 0; i < m_resolution - 1; ++i)
	{
		m_mesh.m_indices[index_iter++] = i * m_resolution;
		m_mesh.m_indices[index_iter++] = (i + 1) * m_resolution;

		for (int j = 0; j < m_resolution - 1; ++j)
		{
			m_mesh.m_indices[index_iter++] = i * m_resolution + j + 1;
			m_mesh.m_indices[index_iter++] = (i + 1) * m_resolution + j + 1;
		}

		m_mesh.m_indices[index_iter++] = 0xFFFF;
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
	else if (gx < 0)
	{
		gx = 0;
		dx = 0;
	}

	if (gy >= m_resolution)
	{
		gy = m_resolution - 2;
		dy = 1.0f;
	}
	else if (gy < 0)
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

	return m_mesh.m_vertices[index].vertex.z();
}

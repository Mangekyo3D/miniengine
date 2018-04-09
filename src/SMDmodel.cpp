#include <fstream>
#include <memory>
#include "SMDmodel.h"
#include "bitmap.h"
#include "render/renderer.h"
#include "render/batch.h"
#include "resourcemanager.h"

using namespace std;

bool SMDModel::openFromFile(ResourceManager* resourceManager, const char *filename)
{
	std::ifstream inputFile(filename, std::ifstream::binary);
	if (!inputFile)
	{
		return false;
	}

	uint32_t numOfMeshes;
	bool bFileHasTexCoords = false;

	m_vertexData.clear();
	m_normalData.clear();
	m_texCoordData.clear();

	m_indexData.clear();

	/*
	This must be implemented by inheritance or/and decorator design pattern
	*/
	char majorVersion, minorVersion;
	inputFile.read(&majorVersion, sizeof(char));
	inputFile.read(&minorVersion, sizeof(char));

	inputFile.read(reinterpret_cast<char *> (&numOfMeshes), sizeof(uint32_t));
	inputFile.read(reinterpret_cast<char *> (&bFileHasTexCoords), sizeof(bool));

	if(bFileHasTexCoords)
	{
		m_bUseTexture = true;

		//Bitmap texture;
		uint32_t pathLength;
		inputFile.read(reinterpret_cast<char *> (&pathLength), sizeof(uint32_t));
		std::unique_ptr <char []> buffer(new char [pathLength+1]);
		inputFile.read(reinterpret_cast<char *> (buffer.get()), pathLength*sizeof(char));
		buffer.get()[pathLength] = '\0';

		m_textures.push_back(resourceManager->loadTexture(buffer.get()));
	}

	uint32_t numOfVertices;

	inputFile.read(reinterpret_cast<char *> (&numOfVertices), sizeof(numOfVertices));

	m_vertexData.reserve(numOfVertices);
	m_normalData.reserve(numOfVertices);

	if (bFileHasTexCoords && m_bUseTexture)
	{
		m_texCoordData.reserve(numOfVertices);
	}

	for (uint32_t i = 0; i < numOfVertices; ++i)
	{
		float data[3];
		inputFile.read(reinterpret_cast<char *> (data), 3 * sizeof(float));
		m_vertexData.emplace_back(data);
		inputFile.read(reinterpret_cast<char *> (data), 3 * sizeof(float));
		m_normalData.emplace_back(data);

		if(bFileHasTexCoords) {
			inputFile.read(reinterpret_cast<char *> (data), 2 * sizeof(float));

			if (m_bUseTexture)
			{
				m_texCoordData.emplace_back(data);
			}
		}
	}

	uint32_t numOfIndices;

	inputFile.read((char *)&numOfIndices, sizeof(numOfIndices));

	m_indexData.reserve(numOfIndices);
	for(uint32_t i = 0; i < numOfIndices; ++i)
	{
		uint32_t data;
		inputFile.read(reinterpret_cast<char *> (&data), sizeof(uint32_t));
		m_indexData.push_back(data);
	}

	inputFile.close();

	m_mesh = createMesh();
	if (m_texCoordData.size() > 0)
	{
		m_pipeline = eDiffuseTextured;
	}
	else
	{
		m_pipeline = eDiffuse;
	}

	return true;
}

CIndexedInstancedBatch* SMDModel::createBatch(Renderer& renderer)
{
	return renderer.addNewBatch <CIndexedInstancedBatch> (m_mesh.get(), m_pipeline, &m_textures);
}

SMDModel::~SMDModel()
{
}

std::unique_ptr<IMesh> SMDModel::createMesh()
{
	if (m_texCoordData.size() > 0)
	{
		auto mesh = std::make_unique <Mesh <VertexFormatVNT>> (static_cast <uint32_t> (m_vertexData.size()), static_cast <uint32_t> (m_indexData.size()));

		for(uint32_t i = 0, totalVerts = static_cast <uint32_t> (m_vertexData.size()); i < totalVerts; ++i)
		{
			VertexFormatVNT& vertex = mesh->m_vertices[i];

			vertex.vertex = m_vertexData[i];
			vertex.normal = m_normalData[i];
			vertex.texCoord = m_texCoordData[i];
		}

		mesh->m_indices.assign(m_indexData.begin(), m_indexData.end());
		return mesh;
	}
	else
	{
		auto mesh = std::make_unique <Mesh <VertexFormatVN>> (static_cast <uint32_t> (m_vertexData.size()), static_cast <uint32_t> (m_indexData.size()));

		for(uint32_t i = 0, totalVerts = static_cast <uint32_t> (m_vertexData.size()); i < totalVerts; ++i)
		{
			VertexFormatVN& vertex = mesh->m_vertices[i];

			vertex.vertex = m_vertexData[i];
			vertex.normal = m_normalData[i];
		}

		mesh->m_indices.assign(m_indexData.begin(), m_indexData.end());

		return mesh;
	}
}

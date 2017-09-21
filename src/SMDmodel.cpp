#include <fstream>
#include <memory>
#include "SMDmodel.h"
#include "bitmap.h"
#include "renderer.h"
#include "resourcemanager.h"
#include "batch.h"

using namespace std;

bool SMDModel::openFromFile(const char *filename)
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
	m_texCoords.clear();

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
		string path = "textures/";
		//Bitmap texture;
		uint32_t pathLength;
		inputFile.read(reinterpret_cast<char *> (&pathLength), sizeof(uint32_t));
		unique_ptr <char []> buffer(new char [pathLength+1]);
		inputFile.read(reinterpret_cast<char *> (buffer.get()), pathLength*sizeof(char));
		*(buffer.get()+pathLength) = '\0';
		path += buffer.get();

//		if(!texture.CreateFromFile(path.c_str()))
//		{
//			m_bUseTexture = false;
//		}
//		else
//		{
//			auto device = IDevice::get <CDevice>();
//			device.glCreateTextures(GL_TEXTURE_2D, 1, &m_imageId);
//			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		}
	}

	uint32_t numOfVertices;

	inputFile.read(reinterpret_cast<char *> (&numOfVertices), sizeof(numOfVertices));

	m_vertexData.reserve(numOfVertices);
	m_normalData.reserve(numOfVertices);

	if (bFileHasTexCoords && m_bUseTexture)
	{
		m_texCoords.reserve(numOfVertices);
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
				m_texCoords.emplace_back(data);
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

	prepareVertexBuffer();

	return true;
}

CBatch* SMDModel::getBatch()
{
	if (!m_batch)
	{
		Renderer& renderer = Renderer::get();
		Material* material = ResourceManager::get().loadMaterial("generic");
		m_batch = renderer.add_mesh_instance(m_mesh.get(), material);
	}

	return m_batch;
}

SMDModel::~SMDModel()
{
}

bool SMDModel::prepareVertexBuffer()
{
	m_mesh = std::make_unique <Mesh> (static_cast <uint32_t> (m_vertexData.size()), static_cast <uint32_t> (m_indexData.size()));

	for(uint32_t i = 0, totalVerts = static_cast <uint32_t> (m_vertexData.size()); i < totalVerts; ++i)
	{
		VertexFormatVN& vertex = m_mesh->m_vertices[i];

		vertex.vertex = m_vertexData[i];
		vertex.normal = m_normalData[i];
	}

	m_mesh->m_indices.assign(m_indexData.begin(), m_indexData.end());

	return true;
}

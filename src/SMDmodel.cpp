#include <fstream>
#include <memory>
#include "SMDmodel.h"
#include "bitmap.h"
#include "render/renderer.h"
#include "render/batch.h"
#include "render/compositingpipeline.h"
#include "resourcemanager.h"

using namespace std;

class SMDModel::MeshAdapter : public IMeshAdapter
{
public:
	MeshAdapter(const SMDModel& parent) : m_parent(parent) {}

	size_t getVertexSize() override { return m_parent.m_texCoordData.size() > 0 ? sizeof(VertexFormatVNT) : sizeof(VertexFormatVN); }
	size_t getNumVertices() override { return m_parent.m_vertexData.size(); }
	size_t getNumIndices() override { return m_parent.m_indexData.size(); };
	size_t getIndexSize() override { return sizeof(m_parent.m_indexData[0]); }
	void fillVertices(uint8_t* buffer) {
		if (m_parent.m_texCoordData.size() > 0)
		{
			VertexFormatVNT* vertBuffer = reinterpret_cast<VertexFormatVNT*>(buffer);
			for (size_t i = 0, totalVerts = m_parent.m_vertexData.size(); i < totalVerts; ++i)
			{
				VertexFormatVNT& vertex = vertBuffer[i];

				vertex.vertex = m_parent.m_vertexData[i];
				vertex.normal = m_parent.m_normalData[i];
				vertex.texCoord = m_parent.m_texCoordData[i];
			}
		}
		else
		{
			VertexFormatVN* vertBuffer = reinterpret_cast<VertexFormatVN*>(buffer);
			for (size_t i = 0, totalVerts = m_parent.m_vertexData.size(); i < totalVerts; ++i)
			{
				VertexFormatVN& vertex = vertBuffer[i];

				vertex.vertex = m_parent.m_vertexData[i];
				vertex.normal = m_parent.m_normalData[i];
			}
		}
	}

	void fillIndices(uint8_t* buffer) override {
		memcpy(buffer, m_parent.m_indexData.data(), getNumIndices() * getIndexSize());
	}

private:
	const SMDModel& m_parent;
};

class ReadHelper {
public:
	ReadHelper(std::ifstream&& stream)
		: m_stream(std::move(stream))
	{}

	template <typename T> T read()
	{
		T value;
		m_stream.read(reinterpret_cast<char*> (&value), sizeof(T));
		return value;
	}

	void readBytes(char* buffer, size_t numBytes) {
		m_stream.read(buffer, numBytes);
	}

private:
	std::ifstream m_stream;
};

std::unique_ptr<SMDModel> SMDModel::openFromFile(ResourceManager* resourceManager, const char* filename)
{
	std::ifstream stream(filename, std::ifstream::binary);
	if (!stream)
		return nullptr;

	ReadHelper reader(std::move(stream));

	auto majorVersion = reader.read<char>();
	auto minorVersion = reader.read<char>();
	auto numOfMeshes = reader.read<uint32_t>();
	auto bFileHasTexCoords = reader.read<bool>();

	std::vector<ITexture*> textures;
	if(bFileHasTexCoords)
	{
		//diffuse texture;
		auto pathLength = reader.read<uint32_t>();
		std::unique_ptr <char []> buffer(new char [pathLength+1]);
		reader.readBytes(buffer.get(), pathLength);
		buffer.get()[pathLength] = '\0';

		if (auto texture = resourceManager->loadTexture(buffer.get()))
			textures.push_back(texture);
	}

	auto numOfVertices = reader.read<uint32_t>();

	std::vector<Vec3> vertexData;
	std::vector<Vec3> normalData;
	std::vector<Vec2> texCoordData;
	std::vector<uint16_t> indexData;

	vertexData.reserve(numOfVertices);
	normalData.reserve(numOfVertices);

	if (bFileHasTexCoords)
		texCoordData.reserve(numOfVertices);

	for (uint32_t i = 0; i < numOfVertices; ++i)
	{
		vertexData.push_back(reader.read<Vec3>());
		normalData.push_back(reader.read<Vec3>());
		if (bFileHasTexCoords)
			texCoordData.push_back(reader.read<Vec2>());
	}

	auto numOfIndices = reader.read<uint32_t>();
	indexData.reserve(numOfIndices);
	for(uint32_t i = 0; i < numOfIndices; ++i)
        indexData.push_back(static_cast<uint16_t>(reader.read<uint32_t>()));

	return std::unique_ptr<SMDModel>(new SMDModel(
		std::move(vertexData), 
		std::move(normalData),
		std::move(texCoordData),
		std::move(indexData),
		std::move(textures)));
}

CIndexedInstancedBatch* SMDModel::createBatch(Renderer& renderer) const
{
	CSceneRenderPass::EScenePipeline pipeline;
	if (m_texCoordData.size() > 0)
		pipeline = CSceneRenderPass::eDiffuseTextured;
	else
		pipeline = CSceneRenderPass::eDiffuse;

	MeshAdapter adapter(*this);
	return renderer.addNewBatch <CIndexedInstancedBatch> (adapter, pipeline, &m_textures);
}

SMDModel::~SMDModel()
{
}

inline SMDModel::SMDModel(std::vector<Vec3>&& vertexData, 
	std::vector<Vec3>&& normalData, 
	std::vector<Vec2>&& texCoordData, 
	std::vector<uint16_t>&& indexData, 
	std::vector<ITexture*>&& textures)
	: m_vertexData(std::move(vertexData))
	, m_normalData(std::move(normalData))
	, m_texCoordData(std::move(texCoordData))
	, m_indexData(std::move(indexData))
	, m_textures(std::move(textures))
{}

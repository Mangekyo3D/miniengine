#pragma once
#include "Util/vertex.h"
#include <vector>
#include <memory>
#include "render/renderer.h"
#include <optional>

class CIndexedInstancedBatch;
class ITexture;
struct IMeshAdapter;
class IPipeline;
class Renderer;
class ResourceManager;

class SMDModel
{
	public:
		~SMDModel();

		static std::unique_ptr<SMDModel> openFromFile(ResourceManager* resourceManager, const char *);
		CIndexedInstancedBatch* createBatch(Renderer& renderer) const;

	private:
		class MeshAdapter;

		SMDModel(std::vector<Vec3>&& vertexData,
				std::vector<Vec3>&& normalData,
				std::vector<Vec2>&& texCoordData,
				std::vector<uint16_t>&& indexData,
				std::vector <ITexture*>&& textures);

		std::vector <Vec3>     m_vertexData;
		std::vector <Vec3>     m_normalData;
		std::vector <Vec2>     m_texCoordData;
		std::vector <uint16_t> m_indexData;
		std::unique_ptr<IMeshAdapter> m_mesh;
		std::vector <ITexture*> m_textures;
};

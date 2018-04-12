#pragma once
#include "Util/vertex.h"
#include <vector>
#include <memory>
#include "render/renderer.h"

class CIndexedInstancedBatch;
class ITexture;
struct IMesh;
class IPipeline;
class Renderer;
class ResourceManager;

class SMDModel
{
	public:
		SMDModel() {}
		~SMDModel();
		bool openFromFile(ResourceManager* resourceManager, const char *);
		CIndexedInstancedBatch* createBatch(Renderer& renderer);
		std::unique_ptr<IMesh> createMesh();

	private:
		std::vector <Vec3>     m_vertexData;
		std::vector <Vec3>     m_normalData;
		std::vector <Vec2>     m_texCoordData;
		std::vector <uint16_t> m_indexData;
		std::unique_ptr<IMesh> m_mesh;
		enum EScenePipeline    m_pipeline;

		bool m_bUseTexture;

		std::vector <ITexture*> m_textures;
};

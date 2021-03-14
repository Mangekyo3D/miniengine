#pragma once
#include <vector>
#include <string>
#include <memory>
#include "icommandbuffer.h"
#include "renderer.h"
#include "compositingpipeline.h"
#include "../Util/vertex.h"

class ICommandBuffer;
class IGPUBuffer;
class ITexture;
class IPipeline;
class IDevice;

/* Mesh - one material per vertex buffer */
struct VertexFormatVN
{
	Vec3     vertex;
	uint32_t normal;
};

struct VertexFormatVNT
{
	Vec3     vertex;
	uint32_t normal;
	Vec2     texCoord;
};

struct VertexFormatV
{
	Vec3     vertex;
};


struct IMeshAdapter
{
	virtual size_t getVertexSize() = 0;
	virtual size_t getNumVertices() = 0;
	virtual size_t getNumIndices() = 0;
	virtual size_t getIndexSize() = 0;
	virtual void fillVertices(uint8_t* buffer) = 0;
	virtual void fillIndices(uint8_t* buffer) = 0;
};

struct MeshInstanceData {
	float modelMatrix[16];
};


// abstract batch, different batch types can use different data/ways to draw
class IBatch
{
	public:
		IBatch(enum CSceneRenderPass::EScenePipeline pipeline) : m_pipeline (pipeline) {}
		virtual ~IBatch() {}
		virtual void draw(ICommandBuffer&) = 0;
		enum CSceneRenderPass::EScenePipeline getPipeline() {return m_pipeline; }

	protected:
		enum CSceneRenderPass::EScenePipeline m_pipeline;
};

class CIndexedInstancedBatch : public IBatch
{
	public:
		CIndexedInstancedBatch(IDevice& device, IMeshAdapter&, enum CSceneRenderPass::EScenePipeline, const std::vector<ITexture*> *textures = nullptr);
		~CIndexedInstancedBatch() override;

		void draw(ICommandBuffer&) override;

		void addMeshInstance(MeshInstanceData& instance);

	private:
		void setupInstanceBuffer(IDevice& device);

		std::vector <MeshInstanceData> m_instanceData;
		std::vector <ITexture*> m_textures;
		size_t m_numIndices;
		bool   m_bShortIndices;
		std::unique_ptr<IGPUBuffer> m_vertexBuffer;
		std::unique_ptr<IGPUBuffer> m_indexBuffer;
		// attributes that are specific to a certain instance
		std::unique_ptr<IGPUBuffer> m_instanceBuffer;
		// number of instances that fit in the instance buffer
		uint32_t m_numInstances;
};


// dynamic batch data need to be filled each frame
class CDynamicArrayBatch : public IBatch
{
	public:
		CDynamicArrayBatch(IDevice& device, enum CSceneRenderPass::EScenePipeline, const std::vector<ITexture*> *textures = nullptr);
		~CDynamicArrayBatch() override;

        void draw(ICommandBuffer& cmd) override;

        void addMeshData(float* data, uint32_t numFloats);

	private:
		std::vector <ITexture*> m_textures;

		// current buffer
		std::unique_ptr<IGPUBuffer> m_vertexBuffer;
        std::vector<float> myData;
};

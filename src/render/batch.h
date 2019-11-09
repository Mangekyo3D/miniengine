#pragma once
#include <vector>
#include <string>
#include <memory>
#include "icommandbuffer.h"
#include "renderer.h"
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


struct IMesh
{
	virtual ~IMesh(){}

	virtual size_t getVertexSize() = 0;
	virtual size_t getNumVertices() = 0;
	virtual void* getVertices() = 0;
	virtual size_t getNumIndices() = 0;
	virtual void* getIndices() = 0;
	virtual size_t getIndexSize() = 0;
};

template <class T, class I = uint16_t> struct Mesh : public IMesh {
		size_t getVertexSize() override {return sizeof(T);}
		size_t getNumVertices() override {return m_vertices.size(); }
		void* getVertices() override {return m_vertices.data(); }
		size_t getNumIndices() override {return m_indices.size(); }
		void* getIndices() override {return m_indices.data(); }
		size_t getIndexSize() override {return sizeof(I);}

		Mesh(uint32_t nov, uint32_t noi)
			: m_vertices(nov)
			, m_indices(noi)
		{
		}

		std::vector <T> m_vertices;
		std::vector <I> m_indices;
};


struct MeshInstanceData {
	float modelMatrix[16];
};


// abstract batch, different batch types can use different data/ways to draw
class IBatch
{
	public:
		IBatch(enum EScenePipeline pipeline) : m_pipeline (pipeline) {}
		virtual ~IBatch() {}
		virtual void draw(ICommandBuffer&) = 0;
		enum EScenePipeline getPipeline() {return m_pipeline; }

	protected:
		enum EScenePipeline m_pipeline;
};

class CIndexedInstancedBatch : public IBatch
{
	public:
		CIndexedInstancedBatch(IDevice& device, IMesh *, enum EScenePipeline, const std::vector<ITexture*> *textures = nullptr);
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
		CDynamicArrayBatch(IDevice& device, enum EScenePipeline, const std::vector<ITexture*> *textures = nullptr);
		~CDynamicArrayBatch() override;

		void draw(ICommandBuffer&) override;

        void addMeshData(float* data, uint32_t numFloats);

	private:
		std::vector <ITexture*> m_textures;

		// current buffer
		std::unique_ptr<IGPUBuffer> m_vertexBuffer;
        uint32_t m_bufferSize;
        std::vector<float> myData;
};

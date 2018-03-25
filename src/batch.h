#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Util/vertex.h"
#include "program.h"


class IGPUBuffer;
class ITexture;

// material descriptor wraps and sets up all data needed for a shader
class IDescriptorInterface
{
	public:
		IDescriptorInterface() 	: m_vertexArrayObject(0) {}
		virtual ~IDescriptorInterface() {}
		virtual void setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf = nullptr, IGPUBuffer* instanceBuf = nullptr) = 0;
		void bind();

	protected:
		uint32_t m_vertexArrayObject;
};

class ArrayDescriptorV : public IDescriptorInterface
{
	public:
		ArrayDescriptorV();
		virtual ~ArrayDescriptorV();
		virtual void setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf = nullptr, IGPUBuffer* instanceBuf = nullptr) override;
};

class IndexedInstancedDescriptorV : public IDescriptorInterface
{
	public:
		IndexedInstancedDescriptorV();
		virtual ~IndexedInstancedDescriptorV();
		virtual void setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf = nullptr, IGPUBuffer* instanceBuf = nullptr) override;
};

class IndexedInstancedDescriptorVT : public IDescriptorInterface
{
	public:
		IndexedInstancedDescriptorVT();
		virtual ~IndexedInstancedDescriptorVT();
		virtual void setVertexStream(IGPUBuffer* vertexBuf, IGPUBuffer* indexBuf = nullptr, IGPUBuffer* instanceBuf = nullptr) override;

	private:
		uint32_t m_sampler;
};

class PipelineObject {
	public:
		PipelineObject(std::string shaderFileName, std::unique_ptr <IDescriptorInterface> descriptor);
		IDescriptorInterface* bind();

	private:
		CProgram m_program;
		std::unique_ptr <IDescriptorInterface> m_descriptor;
};

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
	enum class EPrimitiveType
	{
		eTriangles,
		eTriangleStrip
	};

	IMesh()
		: m_primType(EPrimitiveType::eTriangles)
		, m_bEnablePrimRestart(false)
	{
	}

	virtual size_t getVertexSize() = 0;
	virtual size_t getNumVertices() = 0;
	virtual void* getVertices() = 0;
	virtual size_t getNumIndices() = 0;
	virtual void* getIndices() = 0;
	virtual size_t getIndexSize() = 0;

	EPrimitiveType m_primType;
	bool m_bEnablePrimRestart;
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
		virtual ~IBatch() {}
		virtual void draw() = 0;
};

class CIndexedInstancedBatch : public IBatch
{
	public:
		CIndexedInstancedBatch(IMesh *, PipelineObject *, const std::vector<ITexture*> *textures = nullptr);
		~CIndexedInstancedBatch();

		void draw() override;

		void addMeshInstance(MeshInstanceData& instance);

	private:
		void setupInstanceBuffer();

		std::vector <MeshInstanceData> m_instanceData;
		std::vector <ITexture*> m_textures;
		bool m_bEnablePrimRestart;
		size_t m_numIndices;
		bool   m_bShortIndices;
		IMesh::EPrimitiveType m_primType;
		PipelineObject* m_pipelineState;
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
		CDynamicArrayBatch(PipelineObject *, const std::vector<ITexture*> *textures = nullptr);
		~CDynamicArrayBatch();

		void draw() override;

		void addMeshData();

	private:
		std::vector <ITexture*> m_textures;
		IMesh::EPrimitiveType m_primType;
		PipelineObject* m_material;

		// current buffer
		std::unique_ptr<IGPUBuffer> m_vertexBuffer;
		uint32_t m_bufferSize;
};

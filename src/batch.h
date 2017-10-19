#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Util/vertex.h"
#include "program.h"

class CTexture;

// material descriptor wraps and sets up all data needed for a shader
class IMaterialDescriptor
{
	public:
		virtual ~IMaterialDescriptor() {}
		virtual void setVertexStream(uint32_t vertexBuf, uint32_t indexBuf, uint32_t instanceBuf) = 0;
};

class GenericMaterialDescriptor : public IMaterialDescriptor
{
	public:
		GenericMaterialDescriptor();
		virtual ~GenericMaterialDescriptor();
		virtual void setVertexStream(uint32_t vertexBuf, uint32_t indexBuf, uint32_t instanceBuf);

	private:
		uint32_t m_vertexArrayObject;
};

class TexturedMaterialDescriptor : public IMaterialDescriptor
{
	public:
		TexturedMaterialDescriptor();
		virtual ~TexturedMaterialDescriptor();
		virtual void setVertexStream(uint32_t vertexBuf, uint32_t indexBuf, uint32_t instanceBuf);

	private:
		uint32_t m_vertexArrayObject;
		uint32_t m_sampler;
};

class Material {
	public:
		Material(std::string shaderFileName, std::unique_ptr <IMaterialDescriptor> descriptor);

		IMaterialDescriptor& getDescriptor() { return *m_descriptor.get();}
		void bind();

	private:
		Program m_program;
		std::unique_ptr <IMaterialDescriptor> m_descriptor;
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
		virtual void draw(uint32_t cameraUniformID, uint32_t lightUniformID) = 0;
};

class CBatch : public IBatch
{
	public:
		CBatch(IMesh *, Material *, const std::vector<CTexture*> *textures = nullptr);
		~CBatch();

		void draw(uint32_t cameraUniformID, uint32_t lightUniformID) override;

		void addMeshInstance(MeshInstanceData& instance);

	private:
		void setupInstanceBuffer();

		std::vector <MeshInstanceData> m_instanceData;
		std::vector <CTexture*> m_textures;
		bool m_bEnablePrimRestart;
		size_t m_numIndices;
		IMesh::EPrimitiveType m_primType;
		Material* m_material;
		uint32_t m_vertexBuffer;
		uint32_t m_indexBuffer;
		// attributes that are specific to a certain instance
		uint32_t m_instanceBuffer;
		// number of instances that fit in the instance buffer
		uint32_t m_numInstances;
};


// dynamic batch data need to be filled each frame
class CDynamicBatch : public IBatch
{
	public:
		CDynamicBatch(Material *, const std::vector<CTexture*> *textures = nullptr);
		~CDynamicBatch();

		void draw(uint32_t cameraUniformID, uint32_t lightUniformID);

		void addMeshData();

	private:
		std::vector <CTexture*> m_textures;
		IMesh::EPrimitiveType m_primType;
		Material* m_material;

		// current buffer
		uint32_t m_vertexBuffer;
		uint32_t m_bufferSize;
};

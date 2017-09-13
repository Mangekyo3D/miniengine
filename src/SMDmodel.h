#pragma once

class SMDModel
{
	public:
		SMDModel(): m_vertexData(0), m_indexData(0), m_vertexBufferID(0), m_indexBufferID(0) {}
		~SMDModel();
		void openFromFile(const char *,float scale = 1.0f);
		unsigned int prepareVertexBuffer();
		void render();

	private:
		float* m_vertexData;
		float* m_normalData;
		float* m_texCoords;
		unsigned int* m_indexData;

		bool m_bUseTexture;

		unsigned int m_imageId;
		unsigned int m_samplerId;

		int floatsPerVertex;
		unsigned int m_vertexBufferID;
		unsigned int m_indexBufferID;

		unsigned long m_numOfVertices;
		unsigned long m_numOfIndices;
};

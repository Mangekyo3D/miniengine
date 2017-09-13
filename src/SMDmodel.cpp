#include <fstream>
#include <memory>
#include "SMDmodel.h"
#include "bitmap.h"
#include "cdevice.h"

using namespace std;

#define BUFFER_OFFSET(bytes) ((GLubyte *)NULL + (bytes))

void SMDModel::openFromFile(const char *filename,float scale)
{
	std::ifstream inputFile(filename, std::ifstream::binary);
	if(!inputFile)
		throw 0;
	long numOfMeshes;
	bool fileTexCoords = false;

	delete [] m_vertexData;
	delete [] m_normalData;
	delete [] m_indexData;
	delete [] m_texCoords;

	/*
	This must be implemented by inheritance or/and decorator design pattern
	*/
	char majorVersion, minorVersion;
	inputFile.read(&majorVersion, sizeof(char));
	inputFile.read(&minorVersion, sizeof(char));

	inputFile.read((char *)&numOfMeshes, sizeof(unsigned long));

	inputFile.read((char *)&fileTexCoords, sizeof(bool));
	if(fileTexCoords)
	{
		m_bUseTexture = true;
		string path = "textures/";
		Bitmap texture;
		unsigned long pathLength;
		inputFile.read((char *)&pathLength, sizeof(unsigned long));
		unique_ptr <char> buffer(new char [pathLength+1]);
		inputFile.read((char *)buffer.get(), pathLength*sizeof(char));
		*(buffer.get()+pathLength) = '\0';
		path += buffer.get();

		if(!texture.CreateFromFile(path.c_str()))
		{
			m_bUseTexture = false;
		}
		else
		{
			auto device = IDevice::get <CDevice>();
			device.glCreateTextures(GL_TEXTURE_2D, 1, &m_imageId);
			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			device.glSamplerParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
	}

	inputFile.read((char *)&m_numOfVertices, sizeof(unsigned long));

	if((m_vertexData = new float[3*m_numOfVertices]) == 0)
		throw 0;
	if((m_normalData = new float[3*m_numOfVertices]) == 0)
		throw 0;

	floatsPerVertex = (m_bUseTexture)? 8 : 6;

	if(fileTexCoords && m_bUseTexture)
		if((m_texCoords = new float[2*m_numOfVertices]) == 0)
			throw 0;

	for(unsigned int i = 0; i < m_numOfVertices; i++)
	{
		inputFile.read((char *)(m_vertexData + i*3), 3*sizeof(float));
		m_vertexData[i*3] *= scale;
		m_vertexData[i*3+1] *= scale;
		m_vertexData[i*3+2] *= scale;
		inputFile.read((char *)(m_normalData + i*3), 3*sizeof(float));
		if(fileTexCoords){
			if(m_bUseTexture)
				inputFile.read((char *)(m_texCoords + i*2), 2*sizeof(float));
			else
			{
				float dump[2];
				inputFile.read((char *)dump, 2*sizeof(float));
			}
		}

	}

	inputFile.read((char *)&m_numOfIndices, sizeof(unsigned long));

	if((m_indexData = new unsigned int [m_numOfIndices]) == 0)
		throw 0;
	for(unsigned int i = 0; i < m_numOfIndices; i++)
		inputFile.read((char *)(m_indexData + i), sizeof(unsigned long));

	inputFile.close();

	prepareVertexBuffer();
}

SMDModel::~SMDModel()
{
	delete [] m_vertexData;
	delete [] m_normalData;
	delete [] m_texCoords;
	delete [] m_indexData;
}

GLuint SMDModel::prepareVertexBuffer()
{
	GLfloat *p;

	auto device = IDevice::get <CDevice>();

	if(m_vertexBufferID != 0)
	{
		device.glDeleteBuffers(1, &m_vertexBufferID);
		device.glDeleteBuffers(1, &m_indexBufferID);
	}

	device.glCreateBuffers(1, &m_vertexBufferID);
	device.glCreateBuffers(1, &m_indexBufferID);

	device.glNamedBufferData(m_indexBufferID, m_numOfIndices*sizeof(unsigned int), m_indexData, GL_STATIC_DRAW);
	device.glNamedBufferData(m_vertexBufferID, m_numOfVertices*floatsPerVertex*sizeof(float), NULL, GL_STATIC_DRAW);

	p = (GLfloat *) device.glMapNamedBuffer(m_vertexBufferID, GL_WRITE_ONLY);
	for(unsigned int i = 0; i < m_numOfVertices; i++)
	{
		p[floatsPerVertex*i + 0] = m_vertexData[3*i + 0];
		p[floatsPerVertex*i + 1] = m_vertexData[3*i + 1];
		p[floatsPerVertex*i + 2] = m_vertexData[3*i + 2];
		p[floatsPerVertex*i + 3] = m_normalData[3*i + 0];
		p[floatsPerVertex*i + 4] = m_normalData[3*i + 1];
		p[floatsPerVertex*i + 5] = m_normalData[3*i + 2];
		if(m_bUseTexture)
		{
			p[floatsPerVertex*i + 6] = m_texCoords[2*i + 0];
			p[floatsPerVertex*i + 7] = m_texCoords[2*i + 1];
		}
	}
	device.glUnmapNamedBuffer(m_vertexBufferID);

	return 1;
}


void SMDModel::render()
{
	if (m_vertexData == nullptr)
		return;

	auto device = IDevice::get <CDevice>();

	device.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferID);
	device.glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);

//	device.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, floatsPerVertex * sizeof(float), BUFFER_OFFSET(0));
//	device.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(float), BUFFER_OFFSET(3*sizeof(float)));

	if(m_bUseTexture)
	{
//		device.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(float), BUFFER_OFFSET(6*sizeof(float)));
	}

	device.glDrawRangeElements(GL_TRIANGLES, 0, m_numOfVertices, m_numOfIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

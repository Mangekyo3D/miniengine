#pragma once
#include <string>

class COpenGLShader
{
public:
	enum EShaderType {
		eVertex = 1,
		eFragment = (1 << 1),
		eGeometry = (1 << 2),
		eTesselationEval = (1 << 3),
		eTesselationControl = (1 << 4),
		eCompute = (1 << 5)
	};

	COpenGLShader(std::string filename, EShaderType type);
	~COpenGLShader();

	bool compile();
	void unload();
	EShaderType getType() { return m_type; }
	uint32_t getID() { return m_ID; }

private:
	int shaderTypeToGLType(EShaderType type);

	uint32_t m_ID;
	std::string m_filename;
	EShaderType m_type;
};

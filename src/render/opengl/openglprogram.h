#pragma once
#include "openglshader.h"
#include <map>

class COpenGLProgram
{
public:
	COpenGLProgram();

	void attach(COpenGLShader& shader);
	void use();

	// loading should be done after we have attached all shaders
	bool link();
	void unload();

private:
	std::map <EShaderStage, COpenGLShader*> m_shaders;
	uint32_t m_ID;
};

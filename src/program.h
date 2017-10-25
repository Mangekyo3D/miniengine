#pragma once
#include "shader.h"
#include <map>

class CProgram
{
public:
	CProgram();

	void attach(CShader& shader);
	void use();

	// loading should be done after we have attached all shaders
	bool link();
	void unload();

private:
	std::map <CShader::EType, CShader*> m_shaders;
	uint32_t m_ID;
};

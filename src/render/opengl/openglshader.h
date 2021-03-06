#pragma once
#include "../ipipeline.h"
#include <string>

class COpenGLShader
{
public:
    COpenGLShader(const char* filename, EShaderStage type);
	~COpenGLShader();

	bool compile();
	void unload();
	EShaderStage getType() { return m_type; }
	uint32_t getID() { return m_ID; }

private:
	uint32_t m_ID;
	std::string m_filename;
	EShaderStage m_type;
};

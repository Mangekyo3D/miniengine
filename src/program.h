#ifndef PROGRAM_H
#define PROGRAM_H

#include "shader.h"
#include <map>

class Program
{
public:
	Program();

	void attach(Shader& shader);
	void use();

	// loading should be done after we have attached all shaders
	bool link();
	void unload();

private:
	std::map <Shader::EType, Shader*> m_shaders;
	uint32_t m_ID;
};

#endif // PROGRAM_H

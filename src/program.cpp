#include "program.h"
#include "renderer.h"
#include "cdevice.h"
#include <iostream>

Program::Program()
	: m_ID(0)
{
}

void Program::attach(Shader& shader)
{
	m_shaders[shader.getType()] = &shader;
}

void Program::use()
{
	auto device = IDevice::get <CDevice> ();
	device.glUseProgram(m_ID);
}


bool Program::link()
{
	auto device = IDevice::get <CDevice> ();

	bool bFailedCompilation = false;
	m_ID = device.glCreateProgram();

	// compile all attached shaders
	for (auto iter : m_shaders)
	{
		Shader* shader = iter.second;

		if (!shader->compile())
		{
			bFailedCompilation = true;
		}
		device.glAttachShader(m_ID, shader->getID());
	}

	device.glLinkProgram(m_ID);

	for (auto iter : m_shaders)
	{
		Shader* shader = iter.second;
		device.glDetachShader(m_ID, shader->getID());
	}

	m_shaders.clear();

	if (bFailedCompilation)
	{
		unload();
		return false;
	}

	int linkStatus;
	device.glGetProgramiv(m_ID, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE)
	{
		int logSize;
		device.glGetProgramiv(m_ID, GL_INFO_LOG_LENGTH, &logSize);

		if (logSize > 0)
		{
			char* log = new char [logSize];

			int readSize;
			device.glGetProgramInfoLog(m_ID, logSize, &readSize, log);

			std::cout << log;

			delete [] log;
		}

		unload();
		return false;
	}

	return true;
}

void Program::unload()
{
	if (m_ID)
	{
		auto device = IDevice::get <CDevice> ();
		device.glDeleteProgram(m_ID);
		m_ID = 0;
	}
}

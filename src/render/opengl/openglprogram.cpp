#include "openglprogram.h"
#include "opengldevice.h"
#include <iostream>

COpenGLProgram::COpenGLProgram()
	: m_ID(0)
{
}

void COpenGLProgram::attach(COpenGLShader& shader)
{
	m_shaders[shader.getType()] = &shader;
}

void COpenGLProgram::use()
{
	auto& device = COpenGLDevice::get();
	device.glUseProgram(m_ID);
}


bool COpenGLProgram::link()
{
	auto& device = COpenGLDevice::get();

	bool bFailedCompilation = false;
	m_ID = device.glCreateProgram();

	// compile all attached shaders
	for (auto iter : m_shaders)
	{
		COpenGLShader* shader = iter.second;

		if (!shader->compile())
		{
			bFailedCompilation = true;
		}
		device.glAttachShader(m_ID, shader->getID());
	}

	device.glLinkProgram(m_ID);

	for (auto iter : m_shaders)
	{
		COpenGLShader* shader = iter.second;
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

void COpenGLProgram::unload()
{
	if (m_ID)
	{
		auto& device = COpenGLDevice::get();
		device.glDeleteProgram(m_ID);
		m_ID = 0;
	}
}

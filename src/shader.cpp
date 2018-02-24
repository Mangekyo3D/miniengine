#include "shader.h"
#include "renderer.h"
#include <fstream>
#include <streambuf>
#include <iostream>
#include <memory>
#include "opengldevice.h"
#include "OS/OSFactory.h"

int CShader::shaderTypeToGLType(EType type)
{
	switch (type)
	{
		case EType::eVertex:
			return GL_VERTEX_SHADER;
		case EType::eFragment:
			return GL_FRAGMENT_SHADER;
		case EType::eGeometry:
			return GL_GEOMETRY_SHADER;
		case EType::eCompute:
			return GL_COMPUTE_SHADER;
		case EType::eTesselationEval:
			return GL_TESS_EVALUATION_SHADER;
		case EType::eTesselationControl:
			return GL_TESS_CONTROL_SHADER;
		default:
			break;
	}

	return 0;
}

CShader::CShader(std::string filename, EType type)
	: m_ID (0)
	, m_type(type)
{
	auto& utils =  OSUtils::get();

	switch(type)
	{
		case EType::eFragment:
			filename += ".frag";
			break;
		case EType::eVertex:
			filename += ".vert";
			break;
	}

	m_filename = utils.getShaderPath() + filename;
}

CShader::~CShader()
{
	unload();
}

void CShader::unload()
{
	if (m_ID)
	{
		auto& device = IDevice::get <COpenGLDevice>();
		device.glDeleteShader(m_ID);
		m_ID = 0;
	}
}


bool CShader::compile()
{
	if (m_ID != 0)
	{
		return true;
	}

	std::ifstream file(m_filename);

	if (file)
	{
		auto& device = IDevice::get <COpenGLDevice>();
		std::string source;

		file.seekg(0, std::ios::end);
		source.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		source.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		int shaderType = shaderTypeToGLType(m_type);

		m_ID = device.glCreateShader(shaderType);

		int shaderLength = static_cast <int> (source.length());
		const char *str = source.c_str();

		device.glShaderSource(m_ID, 1, &str, &shaderLength);
		device.glCompileShader(m_ID);

		int status;
		device.glGetShaderiv(m_ID, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			int logSize;
			device.glGetShaderiv(m_ID, GL_INFO_LOG_LENGTH, &logSize);

			if (logSize > 0)
			{
				std::unique_ptr<char[]> log(new char [logSize]);

				int readSize;
				device.glGetShaderInfoLog(m_ID, logSize, &readSize, log.get());

				std::cout << log.get();
			}

			unload();
			return false;
		}

		return true;
	}
	else
	{
		std::cout << "shader file: " << m_filename << " is missing" << std::endl;
	}

	return false;
}

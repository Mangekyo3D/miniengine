#include "shader.h"
#include "renderer.h"
#include <fstream>
#include <streambuf>
#include <iostream>
#include <memory>
#include "cdevice.h"
#include "OS/OSFactory.h"

int Shader::shaderTypeToGLType(EType type)
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

Shader::Shader(std::string filename, EType type)
	: m_ID (0)
	, m_filename(filename)
	, m_type(type)
{
}

Shader::~Shader()
{
	unload();
}

void Shader::unload()
{
	if (m_ID)
	{
		auto device = IDevice::get <CDevice>();
		device.glDeleteShader(m_ID);
		m_ID = 0;
	}
}


bool Shader::compile()
{
	if (m_ID != 0)
	{
		return true;
	}

	auto& utils =  OSUtils::get();
	std::string finalFilename = utils.getShaderPath() + m_filename;
	std::ifstream file(finalFilename);

	if (file)
	{
		auto device = IDevice::get <CDevice>();
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
		std::cout << "shader file: " << finalFilename << " is missing" << std::endl;
	}

	return false;
}
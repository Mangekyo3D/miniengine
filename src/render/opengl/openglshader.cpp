#include <fstream>
#include <streambuf>
#include <iostream>
#include <memory>
#include "openglshader.h"
#include "opengldevice.h"
#include "../../OS/OSFactory.h"

int COpenGLShader::shaderTypeToGLType(EShaderStage type)
{
	switch (type)
	{
		case EShaderStage::eVertexStage:
			return GL_VERTEX_SHADER;
		case EShaderStage::eFragmentStage:
			return GL_FRAGMENT_SHADER;
		case EShaderStage::eGeometryStage:
			return GL_GEOMETRY_SHADER;
		case EShaderStage::eComputeStage:
			return GL_COMPUTE_SHADER;
		case EShaderStage::eTesselationEvalStage:
			return GL_TESS_EVALUATION_SHADER;
		case EShaderStage::eTesselationControlStage:
			return GL_TESS_CONTROL_SHADER;
		default:
			break;
	}

	return 0;
}

COpenGLShader::COpenGLShader(std::string filename, EShaderStage type)
	: m_ID (0)
	, m_type(type)
{
	auto& utils =  OSUtils::get();

	switch(type)
	{
		case EShaderStage::eFragmentStage:
			filename += ".frag.spv";
			break;
		case EShaderStage::eVertexStage:
			filename += ".vert.spv";
			break;
	}

	m_filename = utils.getShaderPath() + filename;
}

COpenGLShader::~COpenGLShader()
{
	unload();
}

void COpenGLShader::unload()
{
	if (m_ID)
	{
		auto& device = COpenGLDevice::get();
		device.glDeleteShader(m_ID);
		m_ID = 0;
	}
}


bool COpenGLShader::compile()
{
	if (m_ID != 0)
	{
		return true;
	}

	std::ifstream file(m_filename, std::ios::in | std::ios::binary);

	if (file)
	{
		auto& device = COpenGLDevice::get();
		std::string source;

		file.seekg(0, std::ios::end);
		source.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		source.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		int shaderType = shaderTypeToGLType(m_type);

		m_ID = device.glCreateShader(shaderType);

		int shaderLength = static_cast <int> (source.length());
		const char *str = source.c_str();

		device.glShaderBinary(1, &m_ID, GL_SHADER_BINARY_FORMAT_SPIR_V, str, shaderLength);
		device.glSpecializeShader(m_ID, "main", 0, nullptr, nullptr);

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

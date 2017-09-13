#include <iostream>
#include "OS/GameWindow.h"
#include "cdevice.h"
#include "batch.h"

IDevice* IDevice::s_device = nullptr;

static void debugCallback(GLenum source,GLenum type,GLuint id,GLenum severity, GLsizei length,const GLchar *message,const void *userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cout << message << std::endl;
		std::cout.flush();
	}
}

template <class T> T initGLfunction(GameWindow& win, T& f, const char *function)
{
	f = reinterpret_cast<T> (win.getGLFunctionPointer(function));
	return f;
}

CDevice::CDevice(GameWindow& win, bool bDebugContext)
{
#define INITFUNCTION(name) initGLfunction(win, name, #name); \
	if (name == nullptr) \
	{ throw 0; }

	INITFUNCTION(glClearNamedFramebufferfv)
	INITFUNCTION(glClearNamedFramebufferfi)

	this->glEnable = ::glEnable;
	this->glDisable = ::glDisable;

	INITFUNCTION(glDrawArrays)

	INITFUNCTION(glAttachShader)
	INITFUNCTION(glCompileShader)
	INITFUNCTION(glAttachShader)
	INITFUNCTION(glDetachShader)
	INITFUNCTION(glCreateProgram)
	INITFUNCTION(glDeleteProgram)
	INITFUNCTION(glCreateShaderProgramv)
	INITFUNCTION(glCreateShader)
	INITFUNCTION(glDeleteShader)
	INITFUNCTION(glCreateProgramPipelines)

	INITFUNCTION(glCreateTextures)
	INITFUNCTION(glCreateSamplers)
	INITFUNCTION(glSamplerParameteri)
	INITFUNCTION(glDrawRangeElements)

	INITFUNCTION(glEnableVertexArrayAttrib)
	INITFUNCTION(glDisableVertexArrayAttrib)
	INITFUNCTION(glGetAttribLocation)

	INITFUNCTION(glGetProgramiv)
	INITFUNCTION(glGetProgramInfoLog)
	INITFUNCTION(glGetShaderiv)
	INITFUNCTION(glGetShaderInfoLog)
	INITFUNCTION(glGetUniformLocation)
	INITFUNCTION(glLinkProgram)
	INITFUNCTION(glUseProgram)
	INITFUNCTION(glShaderSource)
	INITFUNCTION(glUniform1i)
	INITFUNCTION(glUniform2f)
	INITFUNCTION(glUniform1f)

	INITFUNCTION(glCreateFramebuffers)
	INITFUNCTION(glCreateRenderbuffers)
	INITFUNCTION(glBindFramebuffer)
	INITFUNCTION(glBindRenderbuffer)
	INITFUNCTION(glRenderbufferStorage)
	INITFUNCTION(glFramebufferRenderbuffer)
	INITFUNCTION(glFramebufferTexture2D)
	INITFUNCTION(glGenerateMipmap)

	INITFUNCTION(glNamedBufferData)
	INITFUNCTION(glNamedBufferSubData)
	INITFUNCTION(glBindBuffer)
	INITFUNCTION(glBindBufferBase)
	INITFUNCTION(glCreateBuffers)
	INITFUNCTION(glDeleteBuffers)
	INITFUNCTION(glMapNamedBuffer)
	INITFUNCTION(glUnmapNamedBuffer)
	INITFUNCTION(glNamedBufferStorage)
	INITFUNCTION(glMapNamedBufferRange)

	INITFUNCTION(glCreateVertexArrays)
	INITFUNCTION(glBindVertexArray)
	INITFUNCTION(glDeleteVertexArrays)
	INITFUNCTION(glVertexArrayAttribFormat)
	INITFUNCTION(glVertexArrayAttribBinding)
	INITFUNCTION(glVertexArrayVertexBuffer)
	INITFUNCTION(glVertexArrayElementBuffer)

	INITFUNCTION(glPrimitiveRestartIndex)

	if (bDebugContext)
	{
		INITFUNCTION(glDebugMessageCallback)
		INITFUNCTION(glDebugMessageControl)

		glDebugMessageCallback(debugCallback, nullptr);
	}

#undef INITFUNCTION

	// not very nice but meh
	s_device = this;
}

void CDevice::drawBatch(CBatch& batch)
{
}

void CDevice::clearFramebuffer(bool bDepth)
{
	static const float vClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	glClearNamedFramebufferfv(0, GL_COLOR, 0, vClearColor);

	if (bDepth)
	{
		glClearNamedFramebufferfi(0, GL_DEPTH_STENCIL, 0, 1.0f, 0);
	}

	glEnable(GL_DEPTH_TEST);
}

void CDevice::setViewport(uint32_t width, uint32_t height)
{
	glViewport(0, 0, width, height);
}


std::unique_ptr<IDevice> IDevice::createDevice(GameWindow& win, bool bDebugContext)
{
	return std::make_unique<CDevice> (win, bDebugContext);
}
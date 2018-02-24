#include <iostream>
#include "OS/GameWindow.h"
#include "opengldevice.h"
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

COpenGLDevice::COpenGLDevice(GameWindow& win, bool bDebugContext)
{
#define INITFUNCTION(name) initGLfunction(win, name, #name); \
	if (name == nullptr) \
	{ throw 0; }

	this->glEnable = ::glEnable;
	this->glDisable = ::glDisable;
	this->glClear = ::glClear;
	this->glClearDepth = ::glClearDepth;
	this->glViewport = ::glViewport;
	this->glDepthFunc = ::glDepthFunc;
	this->glPixelStorei = ::glPixelStorei;

	INITFUNCTION(glClearNamedFramebufferfv)
	INITFUNCTION(glClearNamedFramebufferfi)

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
	INITFUNCTION(glDeleteTextures)
	INITFUNCTION(glCreateSamplers)
	INITFUNCTION(glDeleteSamplers)
	INITFUNCTION(glSamplerParameteri)
	INITFUNCTION(glTextureStorage2D)
	INITFUNCTION(glTextureSubImage2D)
	INITFUNCTION(glGenerateTextureMipmap)
	INITFUNCTION(glBindTextureUnit)
	INITFUNCTION(glBindSampler)

	INITFUNCTION(glDrawRangeElements)
	INITFUNCTION(glDrawElementsInstanced)

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
	INITFUNCTION(glDeleteFramebuffers)
	INITFUNCTION(glCreateRenderbuffers)
	INITFUNCTION(glBindFramebuffer)
	INITFUNCTION(glBindRenderbuffer)
	INITFUNCTION(glRenderbufferStorage)
	INITFUNCTION(glFramebufferRenderbuffer)
	INITFUNCTION(glNamedFramebufferTexture)
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
	INITFUNCTION(glInvalidateBufferSubData)

	INITFUNCTION(glCreateVertexArrays)
	INITFUNCTION(glBindVertexArray)
	INITFUNCTION(glDeleteVertexArrays)
	INITFUNCTION(glVertexArrayAttribFormat)
	INITFUNCTION(glVertexArrayAttribBinding)
	INITFUNCTION(glVertexArrayVertexBuffer)
	INITFUNCTION(glVertexArrayElementBuffer)
	INITFUNCTION(glVertexArrayBindingDivisor)

	INITFUNCTION(glPrimitiveRestartIndex)

	INITFUNCTION(glDepthRangef)
	INITFUNCTION(glClipControl)

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

#include <iostream>
#include "../../OS/GameWindow.h"
#include "opengldevice.h"
#include "openglbuffer.h"
#ifdef WIN32
#include "openglswapchainwin32.h"
#include "wglext.h"
#else
#include <GL/glx.h>
#include "openglswapchainx11.h"
#endif

IDevice* IDevice::s_device = nullptr;

static void debugCallback(GLenum source,GLenum type,GLuint id,GLenum severity, GLsizei length,const GLchar *message,const void *userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cout << message << std::endl;
		std::cout.flush();
	}
}

template <class T> T initGLfunction(T& f, const char *function)
{
#ifdef WIN32
	f = reinterpret_cast<T> (wglGetProcAddress(function));
#else
	f = reinterpret_cast<T> (glXGetProcAddress((GLubyte *)function));
#endif
	return f;
}

void COpenGLDevice::initialize(GameWindow& win, bool bDebugContext)
{
#ifdef WIN32
	auto swapchain = std::make_unique <COpenGLSwapchainWin32> (win, bDebugContext);
#else
	auto swapchain = std::make_unique <COpenGLSwapchainX11> (win, bDebugContext);
#endif
	win.assignSwapchain(std::move(swapchain));

#define INITFUNCTION(name) initGLfunction(name, #name); \
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

std::unique_ptr<IGPUBuffer> COpenGLDevice::createGPUBuffer(size_t size)
{
	return std::make_unique <COpenGLBuffer>(size);
}

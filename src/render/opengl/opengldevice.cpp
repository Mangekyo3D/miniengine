#include <iostream>
#include "../../OS/GameWindow.h"
#include "opengldevice.h"
#include "openglbuffer.h"
#include "opengltexture.h"
#include "openglpipeline.h"
#include "openglrenderpass.h"
#include "openglcommandbuffer.h"
#ifdef WIN32
#include "openglswapchainwin32.h"
#include "wglext.h"
#else
#include <GL/glx.h>
#include "openglswapchainx11.h"
#endif

COpenGLDevice* COpenGLDevice::s_device = nullptr;

static void APIENTRY debugCallback(GLenum source,GLenum type,GLuint id,GLenum severity, GLsizei length,const GLchar *message,const void *userParam)
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

COpenGLDevice::COpenGLDevice(GameWindow& win, bool bDebugContext)
{
	s_device = this;

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
	this->glCullFace = ::glCullFace;
	this->glFinish = ::glFinish;

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
	INITFUNCTION(glShaderBinary)
	INITFUNCTION(glSpecializeShader)

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
}

std::unique_ptr<ICommandBuffer> COpenGLDevice::beginFrame(ISwapchain& currentSwapchain)
{
	uint32_t width, height;
	currentSwapchain.getSize(width, height);
	return std::make_unique <COpenGLCommandBuffer> (width, height);
}

std::unique_ptr<IGPUBuffer> COpenGLDevice::createGPUBuffer(size_t size, uint32_t usage)
{
	return std::make_unique <COpenGLBuffer>(size, usage);
}

std::unique_ptr<IRenderPass> COpenGLDevice::createRenderPass(SRenderPassParams&)
{
	return std::make_unique <COpenGLRenderPass>();
}

std::unique_ptr<IPipeline> COpenGLDevice::createPipeline(SPipelineParams& params)
{
	auto vertexDescriptor = std::make_unique <COpenGLVertexDescriptorInterface> (params.perDrawBinding, params.perInstanceBinding);
	return std::make_unique <COpenGLPipeline> (params, std::move(vertexDescriptor));
}

std::unique_ptr<ITexture> COpenGLDevice::createTexture(ITexture::EFormat format, uint32_t usage, uint16_t width, uint16_t height, bool bMipmapped)
{
	return std::make_unique <COpenGLTexture>(format, usage, width, height, bMipmapped);
}

void COpenGLDevice::finishJobs()
{
	glFinish();
}

#pragma once
#include "../idevice.h"
#define GL_GLEXT_PROTOTYPES
#define NOMINMAX
#include "glcorearb.h"

class COpenGLDevice : public IDevice
{
public:
	COpenGLDevice(GameWindow& win, bool bDebugContext);

	static COpenGLDevice& get() { return *s_device; }
	virtual std::unique_ptr<ICommandBuffer> beginFrame(ISwapchain& currentSwapchain) override;
	virtual std::unique_ptr<IGPUBuffer> createGPUBuffer(size_t size, uint32_t usage) override;
	virtual std::unique_ptr<IRenderPass> createRenderPass() override;
	virtual std::unique_ptr<IPipeline> createPipeline(SPipelineParams& params, SVertexBinding* perVertBinding, SVertexBinding* perInstanceBinding,
													  const char* shaderName) override;
	virtual std::unique_ptr<ITexture> createTexture(ITexture::EFormat format, uint32_t usage, uint16_t width, uint16_t height, bool bMipmapped) override;

	PFNGLCLEARNAMEDFRAMEBUFFERFVPROC glClearNamedFramebufferfv;
	PFNGLCLEARNAMEDFRAMEBUFFERFIPROC glClearNamedFramebufferfi;

	PFNGLDRAWARRAYSPROC glDrawArrays;
	PFNGLENABLEPROC     glEnable;
	PFNGLDISABLEPROC    glDisable;
	PFNGLCLEARPROC      glClear;
	PFNGLCLEARDEPTHPROC glClearDepth;
	PFNGLVIEWPORTPROC   glViewport;
	PFNGLDEPTHFUNCPROC  glDepthFunc;
	PFNGLPIXELSTOREIPROC glPixelStorei;
	PFNGLCULLFACEPROC   glCullFace;

	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLDETACHSHADERPROC glDetachShader;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
	PFNGLCREATEPROGRAMPIPELINESPROC glCreateProgramPipelines;

	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
	PFNGLGETPROGRAMIVPROC glGetProgramiv;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLSHADERBINARYPROC glShaderBinary;
	PFNGLSPECIALIZESHADERARBPROC glSpecializeShaderARB;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLUNIFORM2FPROC glUniform2f;
	PFNGLUNIFORM1FPROC glUniform1f;

	PFNGLCREATETEXTURESPROC glCreateTextures;
	PFNGLDELETETEXTURESPROC glDeleteTextures;
	PFNGLCREATESAMPLERSPROC glCreateSamplers;
	PFNGLDELETESAMPLERSPROC glDeleteSamplers;
	PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
	PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
	PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
	PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap;
	PFNGLBINDTEXTUREUNITPROC   glBindTextureUnit;
	PFNGLBINDSAMPLERPROC       glBindSampler;

	PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLCREATERENDERBUFFERSPROC glCreateRenderbuffers;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
	PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
	PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
	PFNGLNAMEDFRAMEBUFFERTEXTUREPROC glNamedFramebufferTexture;
	PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLBINDBUFFERBASEPROC glBindBufferBase;
	PFNGLCREATEBUFFERSPROC glCreateBuffers;
	PFNGLDELETEBUFFERSPROC glDeleteBuffers;
	PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
	PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
	PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;
	PFNGLINVALIDATEBUFFERSUBDATAPROC glInvalidateBufferSubData;
	PFNGLMAPNAMEDBUFFERPROC glMapNamedBuffer;
	PFNGLUNMAPNAMEDBUFFERPROC glUnmapNamedBuffer;
	PFNGLMAPNAMEDBUFFERRANGEPROC glMapNamedBufferRange;

	PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
	PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
	PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
	PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
	PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
	PFNGLVERTEXARRAYBINDINGDIVISORPROC glVertexArrayBindingDivisor;
	PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer;
	PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
	PFNGLDISABLEVERTEXARRAYATTRIBPROC glDisableVertexArrayAttrib;

	PFNGLPRIMITIVERESTARTINDEXPROC    glPrimitiveRestartIndex;

	PFNGLDRAWRANGEELEMENTSPROC   glDrawRangeElements;
	PFNGLDRAWELEMENTSINSTANCEDARBPROC glDrawElementsInstanced;

	PFNGLDEPTHRANGEFPROC         glDepthRangef;
	PFNGLCLIPCONTROLPROC         glClipControl;

	PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
	PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl;

private:
	static COpenGLDevice* s_device;
};


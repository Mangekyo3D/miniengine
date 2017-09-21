#pragma once
#include "device.h"
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

class CDevice : public IDevice
{
public:
	CDevice(GameWindow& win, bool bDebugContext);

	void drawBatch(CBatch& batch) override;
	void clearFramebuffer(bool bDepth) override;
	void setViewport(uint32_t width, uint32_t height) override;

	PFNGLCLEARNAMEDFRAMEBUFFERFVPROC glClearNamedFramebufferfv;
	PFNGLCLEARNAMEDFRAMEBUFFERFIPROC glClearNamedFramebufferfi;

	PFNGLDRAWARRAYSPROC glDrawArrays;
	PFNGLENABLEPROC     glEnable;
	PFNGLDISABLEPROC    glDisable;
	PFNGLCLEARPROC      glClear;
	PFNGLCLEARDEPTHPROC glClearDepth;

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
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLUNIFORM2FPROC glUniform2f;
	PFNGLUNIFORM1FPROC glUniform1f;

	PFNGLCREATETEXTURESPROC glCreateTextures;
	PFNGLCREATESAMPLERSPROC glCreateSamplers;
	PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;

	PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers;
	PFNGLCREATERENDERBUFFERSPROC glCreateRenderbuffers;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
	PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
	PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
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

	PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
	PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl;
};


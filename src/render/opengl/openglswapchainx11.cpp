#include "openglswapchainx11.h"
#include "../../OS/X11Window.h"
#include <iostream>

COpenGLSwapchainX11::COpenGLSwapchainX11(GameWindow& win, bool bDebugContext)
{
	X11Window& w = static_cast<X11Window&> (win);
	m_display = w.getDisplay();

	int attribs[] = {//GLX_BUFFER_SIZE, 24,
					 GLX_SAMPLE_BUFFERS, 0,
					 GLX_RENDER_TYPE, GLX_RGBA_BIT,
					 GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					 GLX_X_RENDERABLE, True,
					 GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
					 GLX_CONFIG_CAVEAT, GLX_NONE,
					 GLX_TRANSPARENT_TYPE, GLX_NONE,
					 GLX_DOUBLEBUFFER, True,
					 GLX_STEREO, False,
					 GLX_AUX_BUFFERS, 0,
					 GLX_RED_SIZE, 8,
					 GLX_GREEN_SIZE, 8,
					 GLX_BLUE_SIZE, 8,
					 GLX_ALPHA_SIZE, 0,
					 GLX_DEPTH_SIZE, 0,
					 GLX_STENCIL_SIZE, 8,
					 None};

	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;

	int gl3attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 6,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | (bDebugContext ? GLX_CONTEXT_DEBUG_BIT_ARB : 0),
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		0};

	int numOfConfigs;
	GLXFBConfig *config;
	int tmp;
	XVisualInfo *xvis;

	if (!glXQueryExtension(m_display, &tmp, &tmp))
	{
		std::cout << "No glX support found.\n";
	}

	config = glXChooseFBConfig(m_display, DefaultScreen(m_display), attribs, &numOfConfigs);

	if (numOfConfigs == 0)
	{
		std::cout << numOfConfigs << "No suitable framebuffer configurations found.\n";
	}
	if((xvis = glXGetVisualFromFBConfig(m_display, *config)) == nullptr)
	{
		std::cout << "No corresponding X visual found for framebuffer configuration.\n";
	}

	//This should actually detect the extension on the extension string. Workaround for now...
	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");
	if (glXCreateContextAttribsARB != nullptr)
	{
		if ((m_context = glXCreateContextAttribsARB(m_display, *config, nullptr, True, gl3attribs)) == nullptr)
		{
			std::cout << "OpenGL 4.5 context creation failed.\n";
		}
	}
	else
	{
		std::cout << "OpenGL 4.5 context creation failed.\n";
	}

	m_glxwin = glXCreateWindow(m_display, *config, w.getWindow(), nullptr);

	if (!glXMakeContextCurrent(m_display, m_glxwin, m_glxwin, m_context))
	{
		std::cout << "context binding was not successful.\n";
	}

	XFree(xvis);
	XFree(config);
}

COpenGLSwapchainX11::~COpenGLSwapchainX11()
{
	glXMakeContextCurrent(m_display, None, None, nullptr);
	glXDestroyContext(m_display, m_context);
	glXDestroyWindow(m_display, m_glxwin);
}

void COpenGLSwapchainX11::swapBuffers()
{
	glXSwapBuffers(m_display, m_glxwin);
}

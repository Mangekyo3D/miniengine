#pragma once
#include "../iswapchain.h"
#include <GL/glx.h>

class GameWindow;
class X11Window;

class COpenGLSwapchainX11 : public ISwapchain
{
public:
	COpenGLSwapchainX11(GameWindow&, bool bDebugContext);
	~COpenGLSwapchainX11();
	virtual void swapBuffers() override;

private:
	Display* m_display;
	GLXWindow m_glxwin;
	GLXContext m_context;
};

#pragma once

class X11Window;

#include "GameWindow.h"
#include <GL/glx.h>
#include "../glcorearb.h"

#include <X11/X.h>

class X11Window:public GameWindow
{
	public:
		X11Window(Display*, bool bDebugContext);
		~X11Window();

		virtual void getClientSize(uint32_t& width, uint32_t& height) override;
		virtual void handleOSEvents() override;
		virtual void getMouseState(int& x, int& y) override {}
		virtual void swapBuffers() override { glXSwapBuffers(m_display, m_glxwin); }
		virtual intptr_t getGLFunctionPointer(const char *);
		virtual void maximize() override;

		Display *getDisplay(){return m_display;}
		Window getWindow(){return m_win;}
//		void saveGLScreenShot();

	private:
		Display	*m_display;//encapsulate!!
		Window m_win;
		GLXWindow m_glxwin;
		GLXContext m_context;
};

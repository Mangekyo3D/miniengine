#pragma once
#include "GameWindow.h"
#include <X11/Xlib.h>

class X11Window : public GameWindow
{
	public:
		X11Window(Display*);
		~X11Window();

		virtual void getClientSize(uint32_t& width, uint32_t& height) override;
		virtual void handleOSEvents() override;
		virtual void maximize() override;

		Display* getDisplay(){return m_display;}
		Window getWindow(){return m_win;}
//		void saveGLScreenShot();

	private:
		Display* m_display;//encapsulate!!
		Window m_win;
};

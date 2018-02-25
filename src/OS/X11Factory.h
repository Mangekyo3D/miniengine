#pragma once
#include <X11/Xlib.h>
#include "X11Window.h"
#include "OSFactory.h"

class X11Factory : public OSFactory
{
	public:
		X11Factory()
		{
			m_display = XOpenDisplay(nullptr);
		}
		~X11Factory()
		{
			// this crashes on exit, investigate
			//XCloseDisplay(m_display);
		}

		std::unique_ptr <GameWindow> createGameWindow() const override { return std::make_unique<X11Window> (m_display); }
	protected:
	private:
		Display* m_display;
};


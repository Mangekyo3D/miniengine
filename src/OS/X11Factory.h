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
		std::unique_ptr <GameWindow> createGameWindow(bool bDebugContext) const override { return std::make_unique<X11Window> (m_display, bDebugContext); }
	protected:
	private:
		Display* m_display;
};


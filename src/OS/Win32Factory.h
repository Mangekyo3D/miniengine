#pragma once
#include <windows.h>
#include "OSFactory.h"
#include "Win32Window.h"

class Win32Factory : public OSFactory
{
	public:
		std::unique_ptr <GameWindow> createGameWindow(bool bDebugContext) const override { return std::make_unique <Win32Window>(bDebugContext); }
};


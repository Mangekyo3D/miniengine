#pragma once
#include "../iswapchain.h"
#include <windows.h>

class GameWindow;

class COpenGLSwapchainWin32 : public ISwapchain
{
	public:
		COpenGLSwapchainWin32(GameWindow& win, bool bDebugContext);

		// non copyable
		COpenGLSwapchainWin32(const COpenGLSwapchainWin32&) = delete;
		COpenGLSwapchainWin32& operator = (const COpenGLSwapchainWin32&) = delete;

		~COpenGLSwapchainWin32();
		virtual void swapBuffers() override;

	private:
		HDC m_hdc;
		HGLRC m_hrc;
		HWND m_hWnd;
};

#pragma once
#include "../iswapchain.h"
#include <Windows.h>

class GameWindow;

class COpenGLSwapchainWin32 : public ISwapchain
{
	public:
		COpenGLSwapchainWin32(GameWindow& win, bool bDebugContext);

		// non copyable
		COpenGLSwapchainWin32(const COpenGLSwapchainWin32&) = delete;
		COpenGLSwapchainWin32& operator = (const COpenGLSwapchainWin32&) = delete;

		~COpenGLSwapchainWin32() override;
		virtual void swapBuffers() override;

	private:
		HDC m_hdc;
		HGLRC m_hrc;
		HWND m_hWnd;
};

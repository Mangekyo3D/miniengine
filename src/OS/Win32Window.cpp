#include "Win32Window.h"
#include "gl/GL.h"
#include "wglext.h"
#include "../render/iswapchain.h"

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			CREATESTRUCT* cs = (CREATESTRUCT*) lParam;
			SetWindowLongPtr(hWnd, 0, (LONG_PTR) cs->lpCreateParams);
			break;
		}

		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			KeyEvent event;

			event.type = (msg == WM_KEYDOWN) ? KeyEvent::EType::ePress : KeyEvent::EType::eRelease;

			switch (wParam)
			{
				case VK_ESCAPE:
					event.key = KeyEvent::EKey::eEscape;
					break;

				case VK_SPACE:
					event.key = KeyEvent::EKey::eSpace;
					break;

				case 0x41: // A key
					event.key = KeyEvent::EKey::eAKey;
					break;

				case 0x44: // D key
					event.key = KeyEvent::EKey::eDKey;
					break;

				case 0x53: // S key
					event.key = KeyEvent::EKey::eSKey;
					break;

				case 0x57: // W key
					event.key = KeyEvent::EKey::eWKey;
					break;
			}

			Win32Window* window = (Win32Window*) (GetWindowLongPtr(hWnd, 0));
			window->onKey(event);
			break;
		}

		case WM_MOUSEWHEEL:
		{
			int ticks = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			MouseWheelEvent wheelEvent;
			wheelEvent.ticks = ticks;

			Win32Window* window = (Win32Window*) (GetWindowLongPtr(hWnd, 0));
			window->onMouseWheel(wheelEvent);
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			KeyEvent evt;
			evt.key = KeyEvent::EKey::eLeftMouse;
			evt.type = (msg == WM_LBUTTONDOWN) ? KeyEvent::EType::ePress : KeyEvent::EType::eRelease;
			Win32Window* window = (Win32Window*) (GetWindowLongPtr(hWnd, 0));
			window->onKey(evt);
			break;
		}

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			KeyEvent evt;
			evt.key = KeyEvent::EKey::eRightMouse;
			evt.type = (msg == WM_RBUTTONDOWN) ? KeyEvent::EType::ePress : KeyEvent::EType::eRelease;
			Win32Window* window = (Win32Window*) (GetWindowLongPtr(hWnd, 0));
			window->onKey(evt);
			break;
		}

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_SIZE:
		{
			Win32Window* window = (Win32Window*) (GetWindowLongPtr(hWnd, 0));
			uint32_t width = LOWORD(lParam);
			uint32_t height = HIWORD(lParam);
			ResizeEvent event = {width, height};
			window->onResize(event);
			break;
		}

//		case WM_PAINT:
//		{
//			Win32Window* window = (CWindow*) (GetWindowLongPtr(hWnd, 0));
//			window->swapBuffers();
//			break;
//		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


Win32Window::CWindowClass::CWindowClass()
{
	m_class.style = CS_OWNDC;
	m_class.lpfnWndProc = WndProc;
	m_class.cbClsExtra = 0;
	m_class.cbWndExtra = sizeof(Win32Window*);
	m_class.hInstance = GetModuleHandle(nullptr);
	m_class.hIcon = nullptr;
	m_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_class.hbrBackground = nullptr;
	m_class.lpszMenuName = nullptr;
	m_class.lpszClassName = "SpacePiratesCls";

	RegisterClass(&m_class);
}


Win32Window::CWindowClass::~CWindowClass()
{
	UnregisterClass(m_class.lpszClassName, GetModuleHandle(nullptr));
}

Win32Window::Win32Window()
{
	m_hWnd = CreateWindow(m_wndClass.getName(), "Space Pirates",
						  WS_POPUP, 0, 0,500, 500, nullptr, nullptr, GetModuleHandle(nullptr), this);
	ShowCursor(false);
}

void Win32Window::maximize()
{
	ShowWindow(m_hWnd, SW_MAXIMIZE);
}

Win32Window::~Win32Window()
{
	// first destroy the swapchain itself
	m_swapchain.reset();

	DestroyWindow(m_hWnd);

	ShowCursor(true);
}

void Win32Window::getClientSize(uint32_t& width, uint32_t& height)
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

intptr_t Win32Window::getGLFunctionPointer(const char* function)
{
	return reinterpret_cast<intptr_t> (wglGetProcAddress(function));
}

void Win32Window::handleOSEvents()
{
	MSG message;

	if (PeekMessage(&message, m_hWnd, 0, 0, true))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void Win32Window::getMouseState(int& x, int& y)
{

}

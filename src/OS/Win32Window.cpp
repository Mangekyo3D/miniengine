#include "Win32Window.h"
#include "gl/GL.h"
#include "wglext.h"

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

template <class T> T initWGLfunction(T& f, const char *function)
{
	f = reinterpret_cast<T> (wglGetProcAddress(function));

	return f;
}

Win32Window::Win32Window(bool bDebugContext)
{
	m_hWnd = CreateWindow(m_wndClass.getName(), "Space Pirates",
						  WS_POPUP, 0, 0,500, 500, nullptr, nullptr, GetModuleHandle(nullptr), this);

	HWND dummyWin = CreateWindow(m_wndClass.getName(), "Dummy Window",
								 WS_POPUP, 0, 0,500, 500, nullptr, nullptr, GetModuleHandle(nullptr), this);
	m_hdc = GetDC(m_hWnd);

	PIXELFORMATDESCRIPTOR pfd = {0};

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cGreenBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;

	HDC hdc = GetDC(dummyWin);

	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelFormat, &pfd);

	HGLRC hrc = wglCreateContext(hdc);

	wglMakeCurrent(hdc, hrc);

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
	PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB = nullptr;

#define INITWGLFUNCTION(function) initWGLfunction (function, #function)

	if (INITWGLFUNCTION(wglCreateContextAttribsARB) != nullptr &&
		INITWGLFUNCTION(wglChoosePixelFormatARB) != nullptr)
	{
		uint32_t numFormats;

		int attribList[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, true,
			WGL_DRAW_TO_BITMAP_ARB, false,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_NEED_PALETTE_ARB, false,
			WGL_NEED_SYSTEM_PALETTE_ARB, false,
			WGL_SWAP_LAYER_BUFFERS_ARB, false,
			WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
			WGL_NUMBER_OVERLAYS_ARB, 0,
			WGL_NUMBER_UNDERLAYS_ARB, 0,
			WGL_SHARE_DEPTH_ARB, false,
			WGL_SHARE_STENCIL_ARB, false,
			WGL_SHARE_ACCUM_ARB, false,
			WGL_SUPPORT_GDI_ARB, false,
			WGL_SUPPORT_OPENGL_ARB, true,
			WGL_DOUBLE_BUFFER_ARB, true,
			WGL_STEREO_ARB, false,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_RED_BITS_ARB, 8,
			WGL_GREEN_BITS_ARB, 8,
			WGL_BLUE_BITS_ARB, 8,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_ACCUM_BITS_ARB, 0,
			WGL_ACCUM_RED_BITS_ARB, 0,
			WGL_ACCUM_GREEN_BITS_ARB, 0,
			WGL_ACCUM_BLUE_BITS_ARB, 0,
			WGL_ACCUM_ALPHA_BITS_ARB, 0,
			WGL_DEPTH_BITS_ARB, 0,
			WGL_STENCIL_BITS_ARB, 0,
			WGL_AUX_BUFFERS_ARB, 0,
			0
		};

		if (wglChoosePixelFormatARB(m_hdc, attribList, nullptr, 1, &pixelFormat, &numFormats))
		{
			PIXELFORMATDESCRIPTOR pfdtmp = {0};
			SetPixelFormat(m_hdc, pixelFormat, &pfdtmp);

			int contexAttribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 5,
				WGL_CONTEXT_LAYER_PLANE_ARB, 0,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | (bDebugContext ? WGL_CONTEXT_DEBUG_BIT_ARB : 0),
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};

			m_hrc = wglCreateContextAttribsARB(m_hdc, nullptr, contexAttribs);
			wglMakeCurrent(m_hdc, m_hrc);
		}
		else
		{
			wglMakeCurrent(nullptr, nullptr);
		}
	}
	else
	{
		wglMakeCurrent(nullptr, nullptr);
	}
#undef INITWGLFUNCTION

	wglDeleteContext(hrc);
	ReleaseDC(dummyWin, hdc);
	DestroyWindow(dummyWin);

	ShowWindow(m_hWnd, SW_SHOW);
	ShowCursor(false);
}

void Win32Window::maximize()
{
	ShowWindow(m_hWnd, SW_MAXIMIZE);
}

Win32Window::~Win32Window()
{
	wglMakeCurrent(0, 0);

	wglDeleteContext(m_hrc);
	m_hrc = nullptr;

	ReleaseDC(m_hWnd, m_hdc);
	m_hdc = nullptr;

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

void Win32Window::swapBuffers()
{
	::SwapBuffers(m_hdc);
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

#include "openglswapchainwin32.h"
#include "../../OS/Win32Window.h"
#include "gl/GL.h"
#include "wglext.h"

template <class T> T initWGLfunction(T& f, const char *function)
{
	f = reinterpret_cast<T> (wglGetProcAddress(function));

	return f;
}

COpenGLSwapchainWin32::COpenGLSwapchainWin32(GameWindow& win, bool bDebugContext)
{
	Win32Window& w = static_cast<Win32Window&>(win);

	WNDCLASS dummyclass;
	dummyclass.style = CS_OWNDC;
	dummyclass.lpfnWndProc = DefWindowProc;
	dummyclass.cbClsExtra = 0;
	dummyclass.cbWndExtra = 0;
	dummyclass.hInstance = GetModuleHandle(nullptr);
	dummyclass.hIcon = nullptr;
	dummyclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	dummyclass.hbrBackground = nullptr;
	dummyclass.lpszMenuName = nullptr;
	dummyclass.lpszClassName = "DummyCls";

	RegisterClass(&dummyclass);

	m_hWnd = w.getHandle();
	HWND dummyWin = CreateWindow("DummyCls", "Dummy Window",
								 WS_POPUP, 0, 0,500, 500, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
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
				WGL_CONTEXT_MINOR_VERSION_ARB, 6,
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
	UnregisterClass(dummyclass.lpszClassName, GetModuleHandle(nullptr));
}

COpenGLSwapchainWin32::~COpenGLSwapchainWin32()
{
	wglMakeCurrent(0, 0);

	wglDeleteContext(m_hrc);
	m_hrc = nullptr;

	ReleaseDC(m_hWnd, m_hdc);
	m_hdc = nullptr;
}

void COpenGLSwapchainWin32::swapBuffers()
{
	::SwapBuffers(m_hdc);
}


/*
#ifndef WIN32
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

typedef struct tagBITMAPFILEHEADER {
	WORD	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER,*PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
#endif


void GameWindow::saveGLScreenShot()
{
	std::ofstream file;
	int width;
	int height;
	char *buffer;

	file.open("screenshot.bmp", std::ios::out|std::ios::binary);

	width = getWidth();
	height = getHeight();

	buffer = new char[height*width*3];
	glReadPixels(0 ,0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer);

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	fileHeader.bfType = 0x4D42;
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
		+ width*height*3;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = 54;

	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = 0;
	infoHeader.biXPelsPerMeter =
		0;
	infoHeader.biYPelsPerMeter =
		0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;

	file.write((char *)&fileHeader, sizeof(BITMAPFILEHEADER));
	file.write((char *)&infoHeader, sizeof(BITMAPINFOHEADER));
	file.write(buffer, 3*width*height);

	delete buffer;
}
*/

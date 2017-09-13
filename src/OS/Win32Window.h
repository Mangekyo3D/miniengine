#include "GameWindow.h"
#include <windows.h>

class Win32Window : public GameWindow
{
public:
	Win32Window(bool bDebugContext);
	~Win32Window();

	virtual void getClientSize(uint32_t& width, uint32_t& height) override;

	virtual void swapBuffers() override;

	virtual intptr_t getGLFunctionPointer(const char *) override;

	virtual void handleOSEvents() override;
	virtual void getMouseState(int& x, int& y) override;
	virtual void maximize() override;

private:
	class CWindowClass
	{
	public:
		CWindowClass();
		~CWindowClass();

		const char* getName() { return m_class.lpszClassName; }
	private:
		WNDCLASS m_class;
	};

	CWindowClass m_wndClass;
	HWND m_hWnd;

	HDC m_hdc;
	HGLRC m_hrc;
};

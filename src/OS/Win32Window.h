#include "GameWindow.h"
#include <windows.h>
#include <memory>

class ISwapchain;

class Win32Window : public GameWindow
{
public:
	Win32Window();
	~Win32Window();

	virtual void getClientSize(uint32_t& width, uint32_t& height) override;

	virtual void handleOSEvents() override;
	virtual void getMouseState(int& x, int& y) override;
	virtual void maximize() override;

	HWND getHandle() {return m_hWnd;}
	const char* getWndClassName() const { return m_wndClass.getName(); }

private:
	class CWindowClass
	{
	public:
		CWindowClass();
		~CWindowClass();

		const char* getName() const { return m_class.lpszClassName; }
	private:
		WNDCLASS m_class;
	};

	CWindowClass m_wndClass;
	HWND m_hWnd;
};

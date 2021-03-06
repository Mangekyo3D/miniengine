#include "GameWindow.h"
#include <Windows.h>
#include <memory>

class Win32Window : public GameWindow
{
public:
	Win32Window();
	~Win32Window() override;

	virtual void getClientSize(uint32_t& width, uint32_t& height) override;

	virtual void handleOSEvents() override;
	virtual void maximize() override;
	virtual void getXBoxControllerInput(XBoxInput&) override;

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

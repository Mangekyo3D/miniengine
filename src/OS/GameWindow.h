#pragma once
#include <stdint.h>
#include "../Util/signal.h"

struct ResizeEvent
{
	uint32_t width;
	uint32_t height;
};

struct KeyEvent
{
	enum class EKey
	{
		eLeftMouse,
		eRightMouse,
		eSpace,
		eEscape,
		eWKey,
		eAKey,
		eSKey,
		eDKey,
	};

	enum class EType
	{
		ePress,
		eRelease
	};

	EType type;
	EKey  key;
};

struct MouseWheelEvent
{
	int ticks;
};

class GameWindow
{
	public:
		GameWindow();
		virtual ~GameWindow();
		virtual void getClientSize(uint32_t& width, uint32_t& height) = 0;
		virtual void handleOSEvents() = 0;
		virtual void swapBuffers()=0;
		virtual void maximize() = 0;
		virtual void getMouseState(int& x, int& y) = 0;
		virtual intptr_t getGLFunctionPointer(const char *) = 0;

		bool queryExtension(const char *, const char *);
		//void saveGLScreenShot();

		CSignal <ResizeEvent&>        onResize;
		CSignal <KeyEvent&>           onKey;
		CSignal <MouseWheelEvent&>    onMouseWheel;

	protected:
	private:
};

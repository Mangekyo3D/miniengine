#pragma once
#include <stdint.h>
#include "../Util/signal.h"
#include <memory>

class ISwapchain;

struct ResizeEvent
{
	uint32_t width;
	uint32_t height;
};

struct KeyEvent
{
	enum class EKey
	{
		eNone,
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

	KeyEvent()
		: key(EKey::eNone)
		, type(EType::ePress)
	{
	}

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
		virtual void maximize() = 0;
		virtual void getMouseState(int& x, int& y) = 0;
		void assignSwapchain(std::unique_ptr<ISwapchain> swapchain);
		void swapBuffers();

		CSignal <ResizeEvent&>        onResize;
		CSignal <KeyEvent&>           onKey;
		CSignal <MouseWheelEvent&>    onMouseWheel;

	protected:
		std::unique_ptr <ISwapchain> m_swapchain;
	private:
};

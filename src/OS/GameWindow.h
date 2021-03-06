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

	EKey  key;
	EType type;
};

struct MouseWheelEvent
{
	int ticks;
};

struct XBoxInput {
	float leftRightAxis = 0.0f;
	float upDownAxis = 0.0f;
};

class GameWindow
{
	public:
		GameWindow();
		virtual ~GameWindow();
		virtual void getClientSize(uint32_t& width, uint32_t& height) = 0;
		virtual void handleOSEvents() = 0;
		virtual void maximize() = 0;
		void assignSwapchain(std::unique_ptr<ISwapchain> swapchain);
		ISwapchain& getSwapchain() { return *m_swapchain; }
		void releaseSwapchain();
		virtual void getXBoxControllerInput(XBoxInput&) {}

		CSignal <ResizeEvent&>        onResize;
		CSignal <KeyEvent&>           onKey;
		CSignal <MouseWheelEvent&>    onMouseWheel;

	protected:
		std::unique_ptr <ISwapchain> m_swapchain;
	private:
};

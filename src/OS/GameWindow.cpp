#include "GameWindow.h"
#include <cstring>
#include <fstream>
#include "../render/iswapchain.h"

GameWindow::GameWindow()
{
}

GameWindow::~GameWindow()
{
}

void GameWindow::assignSwapchain(std::unique_ptr<ISwapchain> swapchain)
{
	if (m_swapchain)
	{
		onResize.disconnect(m_swapchain.get());
	}

	m_swapchain = std::move(swapchain);

	onResize.connect(m_swapchain.get(), [this] (ResizeEvent& evt) {
		m_swapchain->onResize(evt.width, evt.height);
	});
}

void GameWindow::releaseSwapchain()
{
	m_swapchain.reset();
}

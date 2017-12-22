#include "engine.h"
#include "effect.h"
#include "OS/OSFactory.h"
#include "ALcontext.h"
#include <ctime>
#include <iostream>
#include "worldentity.h"
#include <cmath>
#include "plane.h"
#include "resourcemanager.h"

Engine Engine::s_engine;

Engine::Engine()
	: m_currentWorldTile(64)
{
}

Engine::~Engine()
{
}

void Engine::startup(SCommandLineOptions& options)
{
	auto& factory = OSFactory::get();
	ResourceManager& resourceManager = ResourceManager::get();

	m_gameWindow = factory.createGameWindow(options.bDebugContext);
	IRenderer::initialize(*m_gameWindow, options.bDebugContext);

	IRenderer& renderer = IRenderer::get();
	resourceManager.initialize();

	m_gameWindow->onResize.connect(this, [this, &renderer] (ResizeEvent& event)
	{
		m_camera.setViewport(event.width, event.height, 0.1f, 100.0f, 30.0f);
		renderer.setViewport(event.width, event.height);
	});

	m_gameWindow->onKey.connect(this, [this] (KeyEvent& event)
	{
		bool pressed = event.type == KeyEvent::EType::ePress;

		switch (event.key)
		{
			case KeyEvent::EKey::eSpace:
			case KeyEvent::EKey::eLeftMouse:
				m_inputState.firePressed = pressed;
				break;
			case KeyEvent::EKey::eAKey:
				m_inputState.leftPressed = pressed;
				break;
			case KeyEvent::EKey::eDKey:
				m_inputState.rightPressed = pressed;
				break;
			case KeyEvent::EKey::eWKey:
				m_inputState.upPressed = pressed;
				break;
			case KeyEvent::EKey::eSKey:
				m_inputState.downPressed = pressed;
				break;
			case KeyEvent::EKey::eEscape:
				m_inputState.menuPressed = pressed;
				break;
		}
	});

	m_gameWindow->onMouseWheel.connect(this, [this] (MouseWheelEvent& event)
	{
		m_inputState.accelarateTick = event.ticks;
	});

	m_gameWindow->maximize();

	IAudioDevice::initialize();
}

void Engine::enterGameLoop()
{
	unsigned int time = 0;

	ResourceManager& resourceManager = ResourceManager::get();
	IRenderer& renderer = IRenderer::get();

	while (true)
	{
		// 60 fps timing
		while ((clock() - time) / static_cast<double> (CLOCKS_PER_SEC) < 0.016)
			;
		time = clock();

		m_inputState.reset();
		m_gameWindow->handleOSEvents();

		if (m_inputState.menuPressed)
		{
			break;
		}

		/* handle physics */
		for(auto& controller : m_controllers)
		{
			controller->update();
		}

		for(int i = static_cast<int> (m_worldEntities.size()) - 1; i >= 0; --i)
		{
			if (m_worldEntities[i]->getActive())
			{
				m_worldEntities[i]->update();
			}
			else
			{
				m_worldEntities[i] = std::move(m_worldEntities.back());
				m_worldEntities.pop_back();
			}
		}

		for(int i = static_cast<int> (m_effects.size()) - 1; i >= 0; --i)
		{
			if(m_effects[i]->getActive())
			{
				m_effects[i]->update();
			}
			else
			{
				m_effects[i] = std::move(m_effects.back());
				m_effects.pop_back();
			}
		}

		// set up camera for the frame
		/* startup, create world */
		m_currentWorldTile.setup_draw_operations();

		m_camera.followFromBehind(*m_playerEntity, 1.0f, 0.2f, 30.0f);

		renderer.updateFrameUniforms(m_camera);
		renderer.drawFrame();

		m_gameWindow->swapBuffers();

		if (!IAudioDevice::get().checkStatus())
		{
			std::cout << "Audio Error" << std::endl;
		}
	}

	resourceManager.cleanup();

	m_effects.clear();
	m_worldEntities.clear();

	IRenderer::shutdown();
	IAudioDevice::shutdown();
}

void Engine::setPlayerEntity(WorldEntity* entity)
{
	m_playerEntity = entity;
}

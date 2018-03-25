#include "engine.h"
#include "effect.h"
#include "audiointerface.h"
#include <ctime>
#include <iostream>
#include "worldentity.h"
#include <cmath>
#include "audiointerface.h"
#include "resourcemanager.h"
#include "OS/GameWindow.h"
#include "render/idevice.h"

Engine::Engine(GameWindow &win, SCommandLineOptions& options)
	: m_currentWorldTile(64)
	, m_gameWindow(&win)
{
	auto device = IDevice::createDevice(win, options.bDebugContext, options.bWithVulkan);
	m_resourceManager = std::make_unique<ResourceManager> (device.get());
	m_renderer =  std::make_unique<Renderer>(m_resourceManager.get(), std::move(device));

	m_gameWindow->onResize.connect(this, &Engine::onResizeEvent);
	m_gameWindow->onKey.connect(this, &Engine::onKeyEvent);
	m_gameWindow->onMouseWheel.connect(this, &Engine::onMouseWheelEvent);

	m_gameWindow->maximize();

	IAudioDevice::initialize();
}

Engine::~Engine()
{
}

void Engine::onKeyEvent(KeyEvent& event)
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
}

void Engine::onMouseWheelEvent(MouseWheelEvent& event)
{
	m_inputState.accelarateTick = event.ticks;
}

void Engine::onResizeEvent(ResizeEvent& event)
{
	m_camera.setViewport(event.width, event.height, 0.1f, 100.0f, 30.0f);
	m_renderer->setViewport(event.width, event.height);
}

void Engine::enterGameLoop()
{
	unsigned int time = 0;

	IAudioDevice& audioDevice = IAudioDevice::get();

	IAudioResource* bgMusic = m_resourceManager->loadAudio("lvl1.wav");
	SAudioInitParams params;
	audioDevice.loopResource(*bgMusic, params);

	while (true)
	{
		// 60 fps timing
		while ((clock() - time) / static_cast<double> (CLOCKS_PER_SEC) < 0.033)
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
			controller->update(*this);
		}

		for(int i = static_cast<int> (m_worldEntities.size()) - 1; i >= 0; --i)
		{
			if (m_worldEntities[i]->getActive())
			{
				m_worldEntities[i]->update(*this);
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
		m_currentWorldTile.setup_draw_operations(m_renderer.get(), m_resourceManager.get());

		m_camera.followFromBehind(*m_playerEntity, 1.0f, 0.2f, 30.0f);

		audioDevice.updateListener(m_playerEntity->getPosition(), m_playerEntity->getObjectToWorldMatrix().getColumn(1), Vec3(0.0f, 0.0f, 0.0f));

		m_renderer->updateFrameUniforms(m_camera);
		m_renderer->drawFrame();

		m_gameWindow->swapBuffers();

		if (!IAudioDevice::get().checkStatus())
		{
			std::cout << "Audio Error" << std::endl;
		}
	}

	m_effects.clear();
	m_worldEntities.clear();
	m_resourceManager.reset();
	m_renderer.reset();

	IAudioDevice::shutdown();
}

void Engine::addEffect(std::unique_ptr<Effect> effect)
{
	m_effects.push_back(std::move(effect));
}

void Engine::setPlayerEntity(WorldEntity* entity)
{
	m_playerEntity = entity;
}

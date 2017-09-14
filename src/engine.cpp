#include "engine.h"
#include "effect.h"
#include "OS/OSFactory.h"
#include "ALcontext.h"
#include <ctime>
#include <iostream>
#include "worldentity.h"
#include <cmath>

/// To be removed once proper controller abstraction is in place
#include "plane.h"

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

	m_gameWindow = factory.createGameWindow(options.bDebugContext);
	m_renderer.initialize(*m_gameWindow, options.bDebugContext);

	m_gameWindow->onResize.connect(this, [this] (ResizeEvent& event)
	{
		m_camera.setViewport(event.width, event.height, 0.1f, 100.0f, 30.0f);
		m_renderer.setViewport(event.width, event.height);
	});

	m_gameWindow->onKey.connect(this, [this] (KeyEvent& event)
	{
		bool pressed = event.type == KeyEvent::EType::ePress;

		switch (event.key) {
			case KeyEvent::EKey::eSpace:
			case KeyEvent::EKey::eLeftMouse:
				m_inputState.firePressed = pressed;
				break;
/*
			case KeyEvent::EKey::eLeftMouse:
				m_inputState.accelaratePressed = pressed;
				break;
			case KeyEvent::EKey::eLeftMouse:
				m_inputState.deccelaratePressed = pressed;
				break;
*/
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

	m_audioDevice = std::make_unique <CNullAudioDevice>();
}

void Engine::enterGameLoop()
{
	unsigned int time = 0;

	/* startup, create world */
	m_currentWorldTile.setup_draw_operations(&m_renderer);

	while (true)
	{
		m_gameWindow->handleOSEvents();

		if (!handleUserInput())
		{
			break;
		}

		// 60 fps timing
		while ((clock() - time) / static_cast<double> (CLOCKS_PER_SEC) < 0.016)
			;
		time = clock();

		/* doAI */

		// set up camera for the frame
		Matrix34 playerEntityTransform = m_playerEntity->getObjectToWorldMatrix();

		m_camera.setPosition(m_playerEntity->getPosition() - 3.0f * playerEntityTransform.getColumn(1));
		m_camera.lookAtWorldPosition(m_playerEntity->getPosition(), playerEntityTransform.getColumn(2));

		m_renderer.updateFrameUniforms(m_camera);
		m_renderer.drawFrame();

		m_gameWindow->swapBuffers();

		/* handle physics */
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

		if (!m_audioDevice->checkStatus())
		{
			std::cout << "Audio Error" << std::endl;
		}
	}

	m_effects.clear();
	m_worldEntities.clear();

	m_renderer.shutdown();
}

void Engine::setPlayerEntity(WorldEntity* entity)
{
	m_playerEntity = entity;
}

bool Engine::handleUserInput()
{
	const float fPitchSpeed = 0.005f;
	const float fRollSpeed = 0.01f;

	Plane* player = static_cast <Plane*> (m_playerEntity);
	if(m_inputState.firePressed)
	{
		player->fire();
	}
	if(m_inputState.leftPressed)
	{
		player->roll(-fRollSpeed);
	}
	if(m_inputState.rightPressed)
	{
		player->roll(fRollSpeed);
	}
	if(m_inputState.upPressed)
	{
		player->pitch(-fPitchSpeed);
	}
	if(m_inputState.downPressed)
	{
		player->pitch(fPitchSpeed);
	}
	if(m_inputState.menuPressed)
	{
		return false;
	}
	if(m_inputState.accelaratePressed)
	{
	}
	if(m_inputState.deccelaratePressed)
	{
	}
	if (m_inputState.accelarateTick != 0)
	{
		float throttle = pow(1.1f, m_inputState.accelarateTick);
		player->accelerate(throttle);
		m_inputState.accelarateTick = 0;
	}

	return true;
}


void Engine::SUserInputState::reset()
{
	firePressed = false;
	accelaratePressed = false;
	deccelaratePressed = false;
	accelarateTick = 0;
	leftPressed = false;
	rightPressed = false;
	upPressed = false;
	downPressed = false;
	menuPressed = false;
}

void Engine::SUserInputState::printDebug()
{
#define DEBUGUSERSTATE(key) std::cout << #key " status: " << ((key) ? "pressed" : "unpressed" ) << std::endl;

	std::cout << std::endl;

	DEBUGUSERSTATE(firePressed);
	DEBUGUSERSTATE(accelaratePressed);
	DEBUGUSERSTATE(deccelaratePressed);
	DEBUGUSERSTATE(leftPressed);
	DEBUGUSERSTATE(rightPressed);
	DEBUGUSERSTATE(upPressed);
	DEBUGUSERSTATE(downPressed);
	DEBUGUSERSTATE(menuPressed);

	std::cout << std::endl;
}

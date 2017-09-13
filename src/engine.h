#pragma once
#include "renderer.h"
#include "worldtile.h"
#include <memory>
#include "camera.h"

class GameWindow;
class OSFactory;
class CAudioDevice;
class WorldEntity;
class Effect;

struct SCommandLineOptions
{
	bool bDebugContext = false;
};

class Engine{
public:
	static Engine& get() { return s_engine; }
	~Engine();

	void startup(SCommandLineOptions& options);
	void enterGameLoop();
	void addWorldEntity(std::unique_ptr<WorldEntity> entity) { m_worldEntities.push_back(std::move(entity)); }
	void addEffect(std::unique_ptr<Effect> effect) { m_effects.push_back(std::move(effect)); }
	void setPlayerEntity(WorldEntity* entity);
	WorldTile& getWorld() { return m_currentWorldTile; }
	std::vector <std::unique_ptr<WorldEntity> >& getEnities() { return m_worldEntities; }
	bool handleUserInput();

private:
	struct SUserInputState
	{
		bool firePressed = false;
		bool accelaratePressed = false;
		bool deccelaratePressed = false;
		int  accelarateTick = 0;
		bool leftPressed = false;
		bool rightPressed = false;
		bool upPressed = false;
		bool downPressed = false;
		bool menuPressed = false;

		void reset();
		void printDebug();
	};

	Engine();

	std::unique_ptr <GameWindow>        m_gameWindow;
	std::unique_ptr <CAudioDevice>      m_audioDevice;

	Renderer                            m_renderer;
	WorldTile                           m_currentWorldTile;

	std::vector <std::unique_ptr<Effect> >      m_effects;
	std::vector <std::unique_ptr<WorldEntity> > m_worldEntities;

	Camera m_camera;
	WorldEntity* m_playerEntity;

	SUserInputState m_inputState;

	static Engine s_engine;
};

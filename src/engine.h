#pragma once
#include "renderer.h"
#include "worldtile.h"
#include <memory>
#include "camera.h"
#include "controller.h"

class GameWindow;
class OSFactory;
class IAudioDevice;
class WorldEntity;
class Effect;
class Controller;

struct KeyEvent;
struct ResizeEvent;
struct MouseWheelEvent;

struct SCommandLineOptions
{
	bool bDebugContext = false;
	bool bWithVulkan = false;
};

class Engine{
public:
	static Engine& get() { return s_engine; }
	~Engine();

	void startup(SCommandLineOptions& options);
	void enterGameLoop();

	void addWorldEntity(std::unique_ptr<WorldEntity> entity) { m_worldEntities.push_back(std::move(entity)); }

	void addController(std::unique_ptr<Controller> controller) { m_controllers.push_back(std::move(controller)); }

	void addEffect(std::unique_ptr<Effect> effect) { m_effects.push_back(std::move(effect)); }
	void setPlayerEntity(WorldEntity* entity);
	WorldEntity& getPlayerEntity() { return *m_playerEntity; }
	WorldTile& getWorld() { return m_currentWorldTile; }
	std::vector <std::unique_ptr<WorldEntity> >& getEnities() { return m_worldEntities; }
	const SUserInputState& getInputState() {return m_inputState; }

private:
	Engine();
	Engine(const Engine&) = delete;

	void onKeyEvent(KeyEvent& event);
	void onMouseWheelEvent(MouseWheelEvent& event);
	void onResizeEvent(ResizeEvent& event);

	std::unique_ptr <GameWindow>        m_gameWindow;

	WorldTile                           m_currentWorldTile;

	std::vector <std::unique_ptr<Effect> >      m_effects;
	std::vector <std::unique_ptr<WorldEntity> > m_worldEntities;
	std::vector <std::unique_ptr<Controller> > m_controllers;

	Camera m_camera;
	WorldEntity* m_playerEntity;

	SUserInputState m_inputState;

	static Engine s_engine;
};

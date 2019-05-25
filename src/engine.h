#pragma once
#include "render/renderer.h"
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
class ResourceManager;

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
	Engine(GameWindow& win, SCommandLineOptions& options);
	Engine(const Engine&) = delete;
	Engine& operator = (const Engine&) = delete;
	~Engine();

	void enterGameLoop();

	void addWorldEntity(std::unique_ptr<WorldEntity> entity) { m_worldEntities.push_back(std::move(entity)); }

	void addController(std::unique_ptr<Controller> controller) { m_controllers.push_back(std::move(controller)); }

	void addEffect(std::unique_ptr<Effect> effect);
	void setPlayerEntity(WorldEntity* entity);
	WorldEntity& getPlayerEntity() { return *m_playerEntity; }
	WorldTile& getWorld() { return m_currentWorldTile; }
	std::vector <std::unique_ptr<WorldEntity> >& getEntities() { return m_worldEntities; }
	const SUserInputState& getInputState() {return m_inputState; }
	Renderer* getRenderer() { return m_renderer.get(); }
	ResourceManager* getResourceManager() { return m_resourceManager.get(); }

private:
	void onKeyEvent(KeyEvent& event);
	void onMouseWheelEvent(MouseWheelEvent& event);
	void onResizeEvent(ResizeEvent& event);

	GameWindow* m_gameWindow;

	WorldTile                           m_currentWorldTile;

	std::vector <std::unique_ptr<Effect> >      m_effects;
	std::vector <std::unique_ptr<WorldEntity> > m_worldEntities;
	std::vector <std::unique_ptr<Controller> > m_controllers;
	std::unique_ptr<Renderer>                 m_renderer;
	std::unique_ptr<ResourceManager>           m_resourceManager;

	Camera m_camera;
	WorldEntity* m_playerEntity;

	SUserInputState m_inputState;
};

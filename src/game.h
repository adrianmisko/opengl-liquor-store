#include "Window.h"
#include "stateManager.h"
#include "dynamicsWorld.h"
#include "resources.h"
#include "gameObject.h"

#include <filesystem>
#include <fstream>
#include <typeinfo>
#include <deque>


struct Game {
	DynamicsWorld dynamicsWorld;
	Window* window;
	StateManager stateManager;
	Resources resources;
	Camera camera;
	std::deque<GameObject> gameObjects;

	Game();
	void run();
private:
	void createGameObjects();
	void loadResources();
};
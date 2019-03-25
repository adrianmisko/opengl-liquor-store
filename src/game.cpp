#include "game.h"

Game::Game() {
	resources = Resources();
	camera = Camera(glm::vec3(0.f, 0.f, 0.f));
	window = new Window(camera);
	stateManager = StateManager();
	loadResources();
	createGameObjects();
}

void Game::run() {
	double lastTime = 0.0;
	double currentTime = 0.0;
	double deltaTime = 0.0;
	while (!glfwWindowShouldClose(window->mWindow)) {
		window->clearBuffers();
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		camera.deltaTime = deltaTime;
		dynamicsWorld.stepSimulation(deltaTime);

		std::cout << camera.position.x << " | " << camera.position.y << " | " << camera.position.z << std::endl;

		for (auto& object : gameObjects)
			object.draw(camera, window);

		window->swapBuffersAndPollEvents();
	}
}

void Game::createGameObjects() {
	int ID, mass;
	float x, y, z;
	std::string type;
	for (auto& p : std::experimental::filesystem::directory_iterator("objectData")) {
		std::cout << p.path() << std::endl;
		std::ifstream file(p.path());
		file >> ID;
		file >> mass;
		file >> x >> y >> z;
		file >> type;
		gameObjects.emplace_back(ID, mass, btVector3(x, y, z), type, resources, dynamicsWorld);
	}	
}

void Game::loadResources() {
	resources.models["box"] = new Model("res/pokeball/pokeball.obj");
	resources.shaders["box"] = new Shader("shaders/model.vs", "shaders/model.fs");
	resources.collisionShapes["box"] = new btBoxShape(btVector3(1.f, 1.f, 1.f));
}

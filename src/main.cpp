#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <bulletWorldImporter/btBulletWorldImporter.h>

#include "dynamicsWorld.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "resources.h"
#include "gameObject.h"
#include "hand.h"
#include "bulletDebugDrawer_OpenGL.h"
#include "particleSystem.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void motorPreTickCallback(btDynamicsWorld *world, btScalar timeStep);

Shader* determinePostprocessingShader(double dt);
void processInput(GLFWwindow *window);
void loadRoom();
void loadResources();
void createGameObjects();

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


double t = 0.0;
double dt = 1.0 / 60.0;
bool clicked = true;
bool moving = false;
bool once = false;
float deltaTime = 2.0f;
float lastFrame = 0.0f;

int ID = 0;
double xxpos;
Hand* hand;
btVector3 rayHitPos;
Shader* red;
ParticleSystem* ps;
int objectHitID = -1;
Resources resources;
std::vector<GameObject> gameObjects;
DynamicsWorld dynamicsWorld;
Camera camera(dynamicsWorld, btVector3(20.f, 0.f, -7.f));

Shader* boxS;

struct ContactCallback : public btCollisionWorld::ContactResultCallback {
	ContactCallback(btVector3* pos) : context(pos) {}
	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {
		btVector3 diff = cp.getPositionWorldOnA() - cp.getPositionWorldOnB();
		btTransform trans = camera.ghostObject->getWorldTransform();
		btVector3 newPos = trans.getOrigin() - diff;
		trans.setOrigin(newPos);
		camera.ghostObject->setWorldTransform(trans); 
		return 0;
	}
	btVector3* context;
};

int main() {

	dynamicsWorld.world->setInternalTickCallback(motorPreTickCallback);

	glewExperimental = true;

	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW\n";
		return -1;
	}


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, 300, 300);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSwapInterval(0);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Failed to initialize GLEW: " << glewGetErrorString(err) << '\n';
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	red = new Shader("basic");
	Shader debug("debug");
	BulletDebugDrawer_OpenGL debugDrawer;

	dynamicsWorld.world->setDebugDrawer(&debugDrawer);
	
	Shader plane("basic");

	float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0.f, 1.f, 0.f), 1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -4.f, 0)));
	btRigidBody::btRigidBodyConstructionInfo
		groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld.world->addRigidBody(groundRigidBody);


	loadResources();
	Shader* postprocessingShader = resources.shaders["none"];
	Shader* particleShader = new Shader("particle");
	Model boxM("res/box/model.obj");
	boxS = new Shader("light");
	loadRoom();
	createGameObjects();
	hand = new Hand(resources, dynamicsWorld, camera);
	ps = new ParticleSystem(camera, particleShader, 300);
	resources.shaders["depth"]->use();
	resources.shaders["depth"]->setFloat("far", 30.0);

	glm::mat4 boxMa(1.f);
	boxMa = glm::translate(boxMa, glm::vec3(-3.5442f, 6.5f, 8.47121f)) * glm::scale(boxMa, glm::vec3(0.5f, 0.1f, 0.5f));

	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
																								  // 
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	double lastTime = 0.0;
	int nbFrames = 0;
	double drinkinTime = 0.0;
	while (!glfwWindowShouldClose(window)) {

		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		camera.deltaTime = deltaTime;
		camera.updateCameraVectors();

		nbFrames++;
		if (currentFrame - lastTime >= 1.0) {
			printf("%f ms/frame, %d fps\n", 1000.0 / double(nbFrames), nbFrames);
			std::cout << "%% " << drinkinTime << std::endl;
			nbFrames = 0;
			lastTime += 1.0;
			moving = false;
			if (drinkinTime > 0) drinkinTime -= 0.1;
		}

		clicked = false;

		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST); // e

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
		

		dynamicsWorld.world->stepSimulation(deltaTime, 5);

		ContactCallback callback(&camera.ghostObject->getWorldTransform().getOrigin());
		dynamicsWorld.world->getCollisionWorld()->contactPairTest(gameObjects.front().rigidBody, camera.ghostObject, callback);
		
		//IFDEF DEBUG 
	    	//debugDrawer.SetMatrices(debug, view, projection);
		//dynamicsWorld.world->debugDrawObject(camera.ghostObject->getWorldTransform(), camera.ghostObject->getCollisionShape(), btVector3(0.1f, 1.f, 0.1f));

		btVector3 out_origin = btVector3(camera.position.x, camera.position.y, camera.position.z);
		btVector3 out_end = out_origin + btVector3(camera.front.x*800.f, camera.front.y*800.f, camera.front.z*800.f);
		btCollisionWorld::ClosestRayResultCallback RayCallback(out_origin, out_end);
		dynamicsWorld.world->rayTest(out_origin, out_end, RayCallback);
		if (RayCallback.hasHit()) {
			rayHitPos = RayCallback.m_hitPointWorld;
		}

		

		if (hand->isDrinking) if (drinkinTime < 45) drinkinTime += deltaTime*2.2f; else drinkinTime += deltaTime*8.f;
		hand->tryToHoldObject(camera);
		hand->setRotation(camera);
		hand->draw(view, projection, camera);

		ps->update(camera);
		ps->draw(view, projection);	

		boxS->use();
		boxS->setMat4("P", projection);
		boxS->setMat4("V", view);
		boxS->setMat4("M", boxMa);
		boxM.draw(boxS);

		glm::mat4 box2Ma = glm::mat4(1.f);
		box2Ma = glm::translate(box2Ma, glm::vec3(20.985f, 6.5f, -16.0351f)) * glm::scale(box2Ma, glm::vec3(0.5f, 0.1f, 0.5f));

		boxS->setMat4("M", box2Ma);
		boxM.draw(boxS);
		
		// should be extracted into separate module & config file
		for (auto& object : gameObjects) {
			if (object.ID == 0 || object.type == "jackdaniels" || object.type == "window") {
				object.shader->use();
				object.shader->setVec3("viewPos", camera.position);
				object.shader->setVec3("pointLights[0].position", glm::vec3(-3.5442f, 6.5f, 8.47121f));
				object.shader->setVec3("pointLights[0].ambient", glm::vec3(0.1f, 0.1f, 0.1f));
				object.shader->setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
				object.shader->setFloat("pointLights[0].quadratic", 0.032f);
				object.shader->setFloat("pointLights[0].linear", 0.09f);
				object.shader->setFloat("pointLights[0].constant", 1.0f);
				object.shader->setVec3("pointLights[1].position", glm::vec3(20.985f, 6.5f, -16.0351f));
				object.shader->setVec3("pointLights[1].ambient", glm::vec3(0.1f, 0.1f, 0.1f));
				object.shader->setVec3("pointLights[1].diffuse", glm::vec3(0.9f, 0.5f, 0.4f));
				object.shader->setFloat("pointLights[1].quadratic", 0.032f);
				object.shader->setFloat("pointLights[1].linear", 0.09f);
				object.shader->setFloat("pointLights[1].constant", 1.0f);
				object.shader->setVec3("dirLight.direction", glm::vec3(-16.5f, 4.f, -4.f));
				object.shader->setVec3("dirLight.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
				object.shader->setVec3("dirLight.diffuse", glm::vec3(0.3f, 0.3f, 0.3f));
			}
			object.draw(view, projection);
		}

		//std::cout << camera.position.x << ' ' << camera.position.y << ' ' << camera.position.z << ' ' << std::endl;	

		glm::mat4 modelP(1.f);
		modelP = glm::translate(modelP, glm::vec3(0.f, -3.f, 0.f)) * glm::scale(modelP, glm::vec3(1000.f, 1000.f, 1000.f)) * glm::rotate(modelP, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
		plane.use();
		plane.setMat4("P", projection);
		plane.setMat4("V", view);
		plane.setMat4("M", modelP);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); 
\
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		postprocessingShader = determinePostprocessingShader(drinkinTime);

		postprocessingShader->use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		clicked = true;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_F)
			if (hand->isHolding)
				if (hand->grabbedObject->type != "lamp")
					hand->isDrinking = true;
		if (key == GLFW_KEY_R)
			for (auto& particle : ps->particles)
				particle.timesUsed = 0;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_F)
			if (hand->isHolding)
				hand->isDrinking = false;
	}
}	


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	xxpos = xpos;
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 
	moving = true;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

void loadRoom() {
	resources.models["room"] = new Model(std::string("res/room/model.obj").c_str());
	btTriangleMesh* meshShape = new btTriangleMesh;
	for (const auto& mesh : resources.models["room"]->meshes) {
		for (int i = 3; i < mesh.indices.size(); i += 3) {
			float x1 = mesh.vertices[mesh.indices[i]].Position.x;
			float y1 = mesh.vertices[mesh.indices[i]].Position.y;
			float z1 = mesh.vertices[mesh.indices[i]].Position.z;
			btVector3 vertex1(x1, y1, z1);
			float x2 = mesh.vertices[mesh.indices[i + 1]].Position.x;
			float y2 = mesh.vertices[mesh.indices[i + 1]].Position.y;
			float z2 = mesh.vertices[mesh.indices[i + 1]].Position.z;
			btVector3 vertex2(x2, y2, z2);
			float x3 = mesh.vertices[mesh.indices[i + 2]].Position.x;
			float y3 = mesh.vertices[mesh.indices[i + 2]].Position.y;
			float z3 = mesh.vertices[mesh.indices[i + 2]].Position.z;
			btVector3 vertex3(x3, y3, z3);
			meshShape->addTriangle(vertex1, vertex2, vertex3);
		}
	}
	resources.collisionShapes["room"] = new btBvhTriangleMeshShape(meshShape, true);
	gameObjects.emplace_back(ID++, 0, "room", "model_lighting", resources, dynamicsWorld);
	resources.models["window"] = new Model(std::string("res/window/model.obj").c_str());
	btTriangleMesh* ms = new btTriangleMesh;
	for (const auto& mesh : resources.models["window"]->meshes) {
		for (int i = 3; i < mesh.indices.size(); i += 3) {
			float x1 = mesh.vertices[mesh.indices[i]].Position.x;
			float y1 = mesh.vertices[mesh.indices[i]].Position.y;
			float z1 = mesh.vertices[mesh.indices[i]].Position.z;
			btVector3 vertex1(x1, y1, z1);
			float x2 = mesh.vertices[mesh.indices[i + 1]].Position.x;
			float y2 = mesh.vertices[mesh.indices[i + 1]].Position.y;
			float z2 = mesh.vertices[mesh.indices[i + 1]].Position.z;
			btVector3 vertex2(x2, y2, z2);
			float x3 = mesh.vertices[mesh.indices[i + 2]].Position.x;
			float y3 = mesh.vertices[mesh.indices[i + 2]].Position.y;
			float z3 = mesh.vertices[mesh.indices[i + 2]].Position.z;
			btVector3 vertex3(x3, y3, z3);
			ms->addTriangle(vertex1, vertex2, vertex3);
		}
	}
	resources.collisionShapes["window"] = new btBvhTriangleMeshShape(ms, true);
	gameObjects.emplace_back(ID++, 0, "window", "transparent", resources, dynamicsWorld);
}

void loadResources() {
	for (auto& p : std::experimental::filesystem::directory_iterator("shaders/")) {
		std::string type = p.path().string().substr(8);
		resources.shaders[type] = new Shader(type);
	}
	for (auto& p : std::experimental::filesystem::directory_iterator("res/")) {
		std::string type = p.path().string().substr(4);
		if (type == "room" || type == "window") continue;
		resources.models[type] = new Model(std::string("res/" + type + "/model.obj").c_str());
		resources.shaders[type] = resources.shaders["depth"];
		btBulletWorldImporter* importer = new btBulletWorldImporter(0);
		importer->loadFile(std::string("res/" + type + "/physics.bullet").c_str());
		resources.collisionShapes[type] = importer->getCollisionShapeByIndex(0);
		delete importer;
	}
}


void createGameObjects() {
	for (auto& p : std::experimental::filesystem::directory_iterator("objects/")) {
		std::string type;
		std::string shader;
		float mass, x, y, z, r, o, t;
		std::ifstream data(p.path());
		data >> type;
		data >> shader;
		data >> mass;
		data >> x;
		data >> y;
		data >> z;
		btVector3 pos(x, y, z);
		std::cout << x << ' ' << y << ' ' << z << std::endl;
		data >> r;
		data >> o;
		data >> t;
		btQuaternion rot(r, o, t, 1);
		btTransform trans(rot, pos);
		gameObjects.emplace_back(ID++, mass, type, shader, trans, resources, dynamicsWorld);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		hand->isHolding = false;
		hand->isDrinking = false;
		hand->grabbedObject = nullptr;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (!hand->isHolding) {
			btVector3 out_origin = btVector3(camera.position.x, camera.position.y, camera.position.z) + btVector3(camera.front.x*7.f, camera.front.y*7.f, camera.front.z*7.f);
			btVector3 out_end = out_origin + btVector3(camera.front.x*8.f, camera.front.y*8.f, camera.front.z*8.f);
			btCollisionWorld::ClosestRayResultCallback RayCallback(out_origin, out_end);
			dynamicsWorld.world->rayTest(out_origin, out_end, RayCallback);
			if (RayCallback.hasHit()) {
				if ((int)RayCallback.m_collisionObject->getUserPointer() == 0) return;
				hand->isHolding = true;
				hand->grabbedObject = &gameObjects[(int)RayCallback.m_collisionObject->getUserPointer()];
			}
		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (hand->isHolding) {
			hand->isHolding = false;
			hand->isDrinking = false;
			_sleep(12);
			btTransform trans;
			trans = hand->grabbedObject->rigidBody->getWorldTransform();
			trans.setOrigin(btVector3(camera.position.x+camera.front.x*10.f, camera.position.y + camera.front.y*10.f, camera.position.z + camera.front.z*10.f));
			hand->grabbedObject->rigidBody->setWorldTransform(trans);
			hand->grabbedObject->rigidBody->applyCentralForce(btVector3(camera.front.x*1200.f, camera.front.y*2000.f, camera.front.z*1200.f));
			hand->grabbedObject = nullptr;
		}
	}
}

void motorPreTickCallback(btDynamicsWorld *world, btScalar timeStep) {
	for (int i = 0; i < camera.ghostObject->getNumOverlappingObjects(); i++) {
		GameObject* object = &gameObjects[(int)camera.ghostObject->getOverlappingObject(i)->getUserPointer()];
		btVector3 camPos = camera.ghostObject->getWorldTransform().getOrigin();
		btVector3 objPos = object->rigidBody->getWorldTransform().getOrigin();
		btVector3 diff = objPos - btVector3(camPos.getX(), 0.f, camPos.getZ());
		object->rigidBody->applyCentralImpulse(diff/2.f);
	}
}

Shader* determinePostprocessingShader(double dt) {
	std::cout << dt << std::endl;
	if (dt < 5)
		return resources.shaders["none"];
	else if (dt < 10)
		return resources.shaders["blur"];
	else if (dt < 15) {
		resources.shaders["sin"]->use();
		resources.shaders["sin"]->setFloat("offset", glfwGetTime() / 1000.0 * 2 * 3.14159 * .75);
		return resources.shaders["sin"];
	}
	else if (dt < 20) {
		resources.shaders["blusin"]->use();
		resources.shaders["blusin"]->setFloat("offset", glfwGetTime() / 1000.0 * 2 * 3.14159 * .75);
		return resources.shaders["blusin"];
	}
	else if (dt < 25)
		return resources.shaders["cross"];
	else if (dt < 30)
		return resources.shaders["fish"];
	else if (dt < 35) {
		resources.shaders["swirl"]->use();
		resources.shaders["swirl"]->setFloat("angle", 1.5f*glm::cos(glfwGetTime()));
		return resources.shaders["swirl"];
	}
	else if (dt < 40)
		return resources.shaders["inverse"];
	else if (dt < 44)
		return resources.shaders["sharpen"];
	else if (dt < 51)
		return resources.shaders["gray"];
	else if (dt < 110) {
		for (auto& obj : gameObjects) {
			obj.shader = resources.shaders["depth"];
		}
		boxS = resources.shaders["depth"];
		ps->shader = resources.shaders["depth"];
		resources.shaders["depth"]->use();
		resources.shaders["depth"]->setFloat("far", dt*5.5);
		return resources.shaders["none"];
	} else {
		return resources.shaders["black"];
	}
}

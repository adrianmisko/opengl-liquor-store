#pragma once

#include "resources.h"
#include "model.h"
#include "shader.h"
#include "dynamicsWorld.h"

#include <fstream>
#include <string>

struct GameObject {
	int ID;
	btScalar mass;
	float modelMatrix[16];
	std::string type;
	btCollisionShape* collisionShape;
	btRigidBody* rigidBody;
	btMotionState* motionState;
	Model* model;
	Shader* shader;

	GameObject(int ID, float mass, const std::string& type, const std::string& shader, btTransform& trans, Resources& res, DynamicsWorld& dynamicsWorld);
	GameObject(int ID, float mass, const std::string& type, const std::string& shader, Resources& res, DynamicsWorld& dynamicsWorld);
	void updateModelMatrix();
	void draw(glm::mat4& view, glm::mat4& projection);
	bool operator==(int oID);
	bool operator==(const GameObject& go);
};

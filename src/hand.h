#pragma once

#include "resources.h"
#include "dynamicsWorld.h"
#include "camera.h"
#include "gameObject.h"

struct Hand {
	int defaultColFlag;
	bool isHolding;
	bool isDrinking;
	float offset;
	float boffset;
	btScalar mass;
	float modelMatrix[16];
	GameObject* grabbedObject;
	btCollisionShape* collisionShape;
	btRigidBody* rigidBody;
	btMotionState* motionState;
	Model* model;
	Shader* shader;

	Hand(Resources& res, DynamicsWorld& dynamicsWorld, Camera& camera);

	void createRigidBody(Camera& camera, DynamicsWorld& dynamicsWorld);
	void updateModelMatrix(Camera& camera);
	void draw(glm::mat4& view, glm::mat4& projection, Camera& camera);
	void setRotation(Camera& camera);
	void tryToHoldObject(Camera& camera);
};
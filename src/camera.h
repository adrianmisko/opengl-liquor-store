#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "dynamicsWorld.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 20.f;
const float SENSITIVITY = 0.08f;
const float ZOOM = 45.0f;

struct Camera {
	btVector3 btPos;
	btGhostObject* ghostObject;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	btScalar mass;
	double deltaTime;
	float yaw;
	float pitch;
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

	Camera(DynamicsWorld& world, btVector3& position, glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f), float yaw=YAW, float pitch=PITCH);
	glm::mat4 GetViewMatrix();
	void ProcessKeyboard(Camera_Movement direction);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void ProcessMouseScroll(float yoffset);
	void createGhostObject(DynamicsWorld& world);
	void updateCameraVectors();
};
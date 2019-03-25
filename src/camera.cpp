#include "camera.h"
#include <iostream>

Camera::Camera(DynamicsWorld& world, btVector3& position, glm::vec3 up, float yaw, float pitch) {
	this->front = glm::vec3(0.0f, 0.0f, -1.0f);
	this->movementSpeed = SPEED;
	this->mouseSensitivity = SENSITIVITY;
	this->zoom = ZOOM;
	this->btPos = position;
	this->position = glm::vec3(btPos.getX(), btPos.getY(), btPos.getZ());
	this->worldUp = up;
	this->yaw = yaw;
	this->pitch = pitch;
	createGhostObject(world);
	updateCameraVectors();
}


glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(position, position+front, up);
}

void Camera::ProcessKeyboard(Camera_Movement direction) {

	btTransform trans = ghostObject->getWorldTransform();
	btPos = trans.getOrigin();

	float velocity = movementSpeed * deltaTime * 0.66f;
	btVector3 movementFront(front.x * velocity, 0.f, front.z * velocity);
	btVector3 movementRight(right.x * velocity, 0.f, right.z * velocity);

	if (direction == FORWARD) {
		btPos = btPos + movementFront;
	}
	if (direction == BACKWARD) {
		btPos = btPos - movementFront;

	}
	if (direction == LEFT) {
		btPos = btPos - movementRight;
	}
		
	if (direction == RIGHT) {
		btPos = btPos + movementRight;
	}

	trans.setOrigin(btPos);
	ghostObject->setWorldTransform(trans);

	updateCameraVectors();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (constrainPitch) {
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
	if (zoom >= 1.0f && zoom <= 45.0f)
		zoom -= yoffset;
	if (zoom <= 1.0f)
		zoom = 1.0f;
	if (zoom >= 45.0f)
		zoom = 45.0f;
}


void Camera::updateCameraVectors() {
	btPos = ghostObject->getWorldTransform().getOrigin();
	this->position.x = btPos.getX();
	this->position.y = btPos.getY() + 4.f;
	this->position.z = btPos.getZ();
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(front, worldUp)); 
	this->up = glm::normalize(glm::cross(right, front));
}

void Camera::createGhostObject(DynamicsWorld& world) {
	ghostObject = new btGhostObject();
	ghostObject->setCollisionShape(new btBoxShape(btVector3(1.2f, 2.3f, 1.2f)));
	ghostObject->setWorldTransform(btTransform(btQuaternion(0.f, 0.f, 0.f, 1.f), btPos));
	world.world->addCollisionObject(ghostObject);
	world.world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	ghostObject->setUserPointer((void*)100);
}
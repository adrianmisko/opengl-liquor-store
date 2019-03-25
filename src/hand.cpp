#include "hand.h"

Hand::Hand(Resources& res, DynamicsWorld& dynamicsWorld, Camera& camera) {
	this->isHolding = false;
	this->isDrinking = false;
	this->grabbedObject = nullptr;
	this->mass = 10;
	this->offset = 6.f;
	this->boffset = 6.f;
	this->shader = res.shaders["depth"];
	this->model = res.models["hand"];
	this->collisionShape = res.collisionShapes["hand"];
	createRigidBody(camera, dynamicsWorld);
}

void Hand::createRigidBody(Camera& camera, DynamicsWorld& dynamicsWorld) {
	glm::vec3 off = camera.position + camera.front * 7.f;
	glm::vec3 off2 = glm::vec3(off.x, off.y - 075.f, off.z);
	btVector3 pos(off2.x, off2.y, off2.z);
	motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), pos));
	btVector3 inertia;
	collisionShape->calculateLocalInertia(mass, inertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, collisionShape, inertia);
	rigidBody = new btRigidBody(rigidBodyCI);
	dynamicsWorld.world->addRigidBody(rigidBody);
	rigidBody->setGravity(btVector3(0.f, 0.f, 0.f));
	defaultColFlag = rigidBody->getCollisionFlags();
}

void Hand::updateModelMatrix(Camera& camera) {
	if (isDrinking) {
		offset = 5.f;
		boffset = 1.7f;
	} else {
		offset = 5.f;
		boffset = 5.f;	
	}
	glm::vec3 off;
	if (!isDrinking)
		off = camera.position + camera.front * offset;
	else
		off = camera.position + glm::vec3(camera.front.x, camera.front.y, -camera.front.z) * offset;
	btVector3 btHandPos = rigidBody->getWorldTransform().getOrigin();
	glm::vec3 handPos = glm::vec3(btHandPos.getX(), btHandPos.getY(), btHandPos.getZ());
	glm::vec3 diff = off - handPos;
	btVector3 force = btVector3(diff.x * 5.f, diff.y * 5.f, diff.z * 5.f);
	rigidBody->setLinearVelocity(force);
	rigidBody->getWorldTransform().getOpenGLMatrix(modelMatrix);
}

void Hand::draw(glm::mat4& view, glm::mat4& projection, Camera& camera) {
	updateModelMatrix(camera);
	shader->use();
	shader->setMat4("V", view);
	shader->setMat4("P", projection);
	shader->setMat4("M", modelMatrix);
	model->draw(shader);
}

void Hand::setRotation(Camera& camera) {
	btTransform trans = rigidBody->getWorldTransform();
	btQuaternion rot;
	btQuaternion dir;
	if (camera.front.z < 0)
		dir = btQuaternion(btVector3(0.f, 1.f, 0.f), -camera.front.x);
	else 
		dir = btQuaternion(btVector3(0.f, 1.f, 0.f), camera.front.x) *  btQuaternion(btVector3(0.f, 1.f, 0.f), 3.f);
	rot = btQuaternion(btVector3(1.f, 0.f, 0.f), -camera.front.y) * dir;
	trans.setRotation(rot);
	rigidBody->setWorldTransform(trans);
}

void Hand::tryToHoldObject(Camera& camera) {
	if (grabbedObject == nullptr || !isHolding)
		return;
	btVector3 diff;
	btVector3 objectPos = grabbedObject->rigidBody->getWorldTransform().getOrigin();
	if (isDrinking) {
		glm::vec3 gdiff = glm::vec3(camera.position.x, camera.position.y - 0.9f, camera.position.z) + camera.front * boffset;
		diff = btVector3(gdiff.x, gdiff.y, gdiff.z) - objectPos;
	} else
		diff = rigidBody->getWorldTransform().getOrigin() - objectPos;
	btVector3 force = diff * 5.f;
	grabbedObject->rigidBody->setLinearVelocity(force);
	btTransform trans = grabbedObject->rigidBody->getWorldTransform();
	btQuaternion rot;
	btQuaternion dir;
	if (camera.front.z < 0)
		dir = btQuaternion(btVector3(0.f, 1.f, 0.f), -camera.front.x);
	else
		dir = btQuaternion(btVector3(0.f, 1.f, 0.f), camera.front.x) *  btQuaternion(btVector3(0.f, 1.f, 0.f), 3.f);
	if (isDrinking)
		dir = dir * btQuaternion(btVector3(1.f, 0.f, 0.f), 1.f);
	rot = dir;
	trans.setRotation(rot);
	grabbedObject->rigidBody->setWorldTransform(trans);
}

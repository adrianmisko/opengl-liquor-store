#include "gameObject.h"

GameObject::GameObject(int ID, float mass, const std::string & type, const std::string& shader, btTransform& trans, Resources & res, DynamicsWorld & dynamicsWorld) {
	this->ID = ID;
	this->mass = mass;
	this->shader = res.shaders[shader];
	this->model = res.models[type];
	this->type = type;
	this->collisionShape = res.collisionShapes[type];
	btVector3 ineria(0.f, 0.f, 0.f);
	if (mass == 0)
		ineria = btVector3(0.f, 0.f, 0.f);
	else
		collisionShape->calculateLocalInertia(mass, ineria);
	btDefaultMotionState* motionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfo(mass, motionState, collisionShape, ineria);
	rigidBody = new btRigidBody(rigidBodyConstructionInfo);
	dynamicsWorld.world->addRigidBody(rigidBody);
	rigidBody->setUserPointer((void*)ID);
}

GameObject::GameObject(int ID, float mass, const std::string & type, const std::string& shader, Resources & res, DynamicsWorld & dynamicsWorld) {
	this->ID = ID;
	this->mass = mass;
	this->shader = res.shaders[shader];
	this->model = res.models[type];
	this->type = type;
	this->collisionShape = res.collisionShapes[type];
	btVector3 ineria(0.f, 0.f, 0.f);
	if (mass == 0)
		ineria = btVector3(0.f, 0.f, 0.f);
	else
		collisionShape->calculateLocalInertia(mass, ineria);
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0.f, 0.f, 0.f)));
	btRigidBody::btRigidBodyConstructionInfo rigidBodyConstructionInfo(mass, motionState, collisionShape, ineria);
	rigidBody = new btRigidBody(rigidBodyConstructionInfo);
	dynamicsWorld.world->addRigidBody(rigidBody);
	rigidBody->setUserPointer((void*)ID);
}


void GameObject::updateModelMatrix() {
	rigidBody->getWorldTransform().getOpenGLMatrix(modelMatrix);
}

void GameObject::draw(glm::mat4& view, glm::mat4& projection) {
	updateModelMatrix();
	shader->use();
	shader->setMat4("V", view);
	shader->setMat4("P", projection);
	shader->setMat4("M", modelMatrix);
	model->draw(shader);
}

bool GameObject::operator==(int oID) {
	if (this->ID == oID) return true; else return false;
}

bool GameObject::operator==(const GameObject & go) {
	if (this->ID == go.ID) return true; else return false;
}

#include "dynamicsWorld.h"

DynamicsWorld::DynamicsWorld() {
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, 
		broadphase, solver, collisionConfiguration);
	world->setGravity(btVector3(0.f, -19.6f, 0.f));
}

void DynamicsWorld::setGravity(const btVector3& vec) {
	world->setGravity(vec);
}

void DynamicsWorld::stepSimulation(double dt) {
	world->stepSimulation(dt, 5);
}

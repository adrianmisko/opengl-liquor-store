#pragma once

#include <btBulletDynamicsCommon.h>

struct DynamicsWorld {
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* world;

	DynamicsWorld();
	void setGravity(const btVector3& vec);
	void stepSimulation(double dt);
};


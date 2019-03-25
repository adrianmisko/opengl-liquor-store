#pragma once

#include <btBulletDynamicsCommon.h>

#include "shader.h"
#include "model.h"

#include <map>
#include <string>

struct Resources {
	std::map<std::string, Shader*> shaders;
	std::map<std::string, Model*> models;
	std::map<std::string, btCollisionShape*> collisionShapes;
};
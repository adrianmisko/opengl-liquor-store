#pragma once

#include <glm/glm.hpp>

struct Particle {
	int timesUsed;
	glm::vec3 pos, velocity;
	glm::vec4 color;
	float size;
	float gravity;
	float ttl;
	Particle();
};


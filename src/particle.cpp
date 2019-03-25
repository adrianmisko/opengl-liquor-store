
#include "particle.h"

Particle::Particle() {
	this->size = rand() / double(RAND_MAX) / 8 + 0.2f;
	this->pos = glm::vec3(1000.f, 1000.f, 1000.f);
	this->color = glm::vec4(rand() / double(RAND_MAX), rand() / double(RAND_MAX), rand() / double(RAND_MAX), 1.f);
	this->velocity = glm::vec3(0.f);
	this->gravity = (rand() / double(RAND_MAX)) * 5.f + 3.f;
	this->ttl = (rand() / (double)RAND_MAX) * 2.f;
	timesUsed = 10000;
}


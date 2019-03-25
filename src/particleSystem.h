#pragma once

#include "camera.h"
#include "particle.h"
#include "shader.h"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <vector>

struct ParticleSystem {
	unsigned int VBO, VAO;
	int maxParticles;
	int lastUsedParticle;
	Shader* shader;
	glm::vec3 position;
	glm::vec3 front;
	std::vector<Particle> particles;

	ParticleSystem(Camera& camera, Shader* shader, int maxParticles);
	void init();
	void update(Camera& camera);
	void draw(glm::mat4& view, glm::mat4& projection);
	void spawnParticle(Particle& particle);
};


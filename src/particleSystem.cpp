#include "particleSystem.h"

#include "glm/ext.hpp"


ParticleSystem::ParticleSystem(Camera& camera, Shader* shader, int maxParticles) {
	this->maxParticles = maxParticles;
	this->shader = shader;
	particles.reserve(maxParticles);
	this->front = camera.front;
	this->position = camera.position;
	this->lastUsedParticle = 0;
	init();
}

void ParticleSystem::init() {
	float vertices[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	for (int i = 0; i < maxParticles; i++)
		particles.push_back(Particle());
}

void ParticleSystem::update(Camera& camera) {

	position = camera.position;
	front = camera.front;

	for (int i = 0; i < maxParticles; i++) {
		Particle &p = particles[i];
		p.ttl -= camera.deltaTime;
		if (p.ttl > 0) {
			p.timesUsed++;
			p.velocity = p.velocity + glm::vec3(0.f, p.gravity, 0.f) * camera.deltaTime * 0.5f;
			p.pos -= p.velocity * (float)camera.deltaTime;
			if (p.pos.y < 0)
				p.pos.y = 0.f;
		} else if (p.timesUsed < 200) {
			spawnParticle(p);
		} else if (p.timesUsed >= 200) {
			p.pos += glm::vec3(0.f, -p.gravity, 0.f) * camera.deltaTime * 0.5f;
			if (p.pos.y < -2.2f)
				p.pos.y = -0.5f;
		}
	}
}


void ParticleSystem::draw(glm::mat4& view, glm::mat4& projection) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shader->use();
	shader->setMat4("V", view);
	shader->setMat4("P", projection);
	for (auto& particle : particles) {
		glm::mat4 model(1.f);
		model = glm::translate(model, particle.pos) * glm::scale(model, glm::vec3(particle.size, particle.size, particle.size));
		shader->setMat4("M", model);
		shader->setVec3("col", glm::vec3(particle.color.r, particle.color.g, particle.color.b));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	std::cout << particles[0].color.r << ' ' << particles[0].color.g << ' ' << particles[0].color.a << '\n';
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void ParticleSystem::spawnParticle(Particle & particle) {
	float minusone = (rand() / (double)RAND_MAX) - (rand() / (double)RAND_MAX);
	particle.pos = glm::vec3(position.x, position.y-1.5f, position.z);
	particle.velocity = glm::vec3(front.x * minusone*20.f, 0.f, -front.z*(rand()/(double)RAND_MAX)*18.f);
	particle.ttl = (rand() / (double)RAND_MAX) * 2.f;
}

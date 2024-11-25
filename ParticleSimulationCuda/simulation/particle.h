#pragma once

#include <glm/glm.hpp>




// contains basic information for each particle
struct Particle {

	float radius;
	glm::vec3 scale;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 new_acceleration;
	float dt = 1.f / 120.f; //temporary solution, supposed to be tied to the frame and update rate

	Particle(float r, glm::vec3 p, glm::vec3 v) {
		radius = r;
		position = p;
		velocity = v;
		acceleration = glm::vec3(0.f);
		new_acceleration = glm::vec3(0.f);
		scale = glm::vec3(r);
	}

	// verlet integration
	void forces_verlet() {
		position = position + velocity * dt + acceleration * 0.5f * dt * dt;
		velocity = velocity + (acceleration + new_acceleration) * (dt * 0.5f);

		acceleration = new_acceleration;
		new_acceleration = glm::vec3(0.f);
	}

	void move() {
		position = position + velocity;
	}
};


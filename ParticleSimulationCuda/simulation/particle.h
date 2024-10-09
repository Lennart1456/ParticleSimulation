#pragma once

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Particle {

	float radius;
	glm::vec3 scale;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 new_acceleration;
	float dt = 1.f / 120.f;

	Particle(float r, glm::vec3 p, glm::vec3 v) {
		radius = r;
		position = p;
		velocity = v;
		acceleration = glm::vec3(0.f);
		new_acceleration = glm::vec3(0.f);
		scale = glm::vec3(r);
	}

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


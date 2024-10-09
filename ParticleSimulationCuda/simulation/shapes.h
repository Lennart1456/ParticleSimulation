#pragma once

#pragma once

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Circle {

	int resolution;
	glm::vec3 position;
	glm::vec3 center;
	std::vector<float> vertices;

	Circle(int res) {
		resolution = res;
		center = glm::vec3(0.f, 0.f, 0.f);
		init_vertices();
	}

	void init_vertices() {
		const float PI = acos(-1.0f);
		float angleStep = 2 * PI / resolution;
		float angle;
		float vertice_x;
		float vertice_y;

		vertices.clear();

		//circle center as first vertex, for triangle_fan primitive
		vertices.push_back(0.0);
		vertices.push_back(0.0);
		vertices.push_back(0.0);

		for (int i = 0; i < resolution; i++)
		{
			angle = i * angleStep;

			vertice_x = cosf(angle);
			vertice_y = sinf(angle);

			vertices.push_back(vertice_x);
			vertices.push_back(vertice_y);
			vertices.push_back(0.0);
		};
		vertices.push_back(vertices[3]);
		vertices.push_back(vertices[4]);
		vertices.push_back(vertices[5]);
	}
};

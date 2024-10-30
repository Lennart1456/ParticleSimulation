#pragma once

#include <vector>
#include <random>
#include "particle.h"
#include "BarnesHut.h"
#include <thread>
#include <iostream>



struct Particlesystem {
	int amount;
	const float gravitational_constant = 0.06743f;
	std::vector<Particle> particles;

	bool collision_on;
	bool gravity_on;

	glm::vec3 center_mass;
	glm::vec3 center_mass_vel;
	float total_mass;

	float energy;

	Quadtree Qtree;


	Particlesystem(int n, bool g, bool c){
		amount = n;
		gravity_on = g;
		collision_on = c;
		spawn();
		calc_center_mass();
	}

	//spawns random particles, only sets the position, velocity is 0
	void spawn() {

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 gen(rd()); // seed the generator
		std::uniform_real_distribution<> distr(-1.0, 1.0); // define the range

		float r = distr(gen);
		float x = distr(gen);
		float y = distr(gen);
		float z = distr(gen);
		float vx = 0;
		float vy = 0;

		for (int i = 0; i < amount; i++) {

			r = distr(gen);
			x = distr(gen);
			y = distr(gen);
			z = distr(gen);
			vx = distr(gen);
			vy = distr(gen);
			
			Particle p(0.01f, glm::vec3(x * 10.f, y * 10.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
			particles.push_back(p);
		}
	}

	//unused
	void calc_center_mass() {
		glm::vec3 weighted_position(0.f);
		glm::vec3 weighted_velocity(0.f);

		for (Particle &p : particles) {
			weighted_position += p.radius * p.position;
			weighted_velocity += p.velocity * p.radius;
		}
		center_mass = (1.f / total_mass) * weighted_position;
		center_mass_vel = weighted_velocity / total_mass;
	}

	//unused
	void calc_energy() {
		float e_pot = 0.f;
		float e_kin = 0.f;

		for (Particle& p : particles) {
			e_pot += p.radius * glm::length(p.acceleration) * glm::length(p.position - center_mass);
			e_kin += p.radius * glm::length(p.velocity);
		}

		energy = e_pot + e_kin;
		std::cout << "Energy: " << energy << "Potential: " << e_pot << "Kinetic: " << e_kin << std::endl;
	}

	//unused
	void calc_angular_momentum() {

		float linear_momentum = total_mass * glm::length(center_mass_vel);
		float angular_momentum = 0.f;

		glm::vec3 cross_position;

		for (Particle& p : particles) {
			glm::vec3 relative_position = p.position - center_mass;

			glm::vec3 mass_position = p.radius * relative_position;
			glm::vec3 position_change = relative_position;
		}
	}

	// creates Quadtree, calculates the forces based on it and calculates the new velocity of the particles
	void update() {
		barnes_hut_multi();

		for (Particle &p : particles) {
			p.forces_verlet();
		}

		// Vector is emptied, memory is still allocated
		Qtree.nodes.clear();
		Qtree.init_root_node();
	}

	// barnes hut single thread
	void barnes_hut() {

		//construct the tree
		for (int i = 0; i < amount; i++) {
			Qtree.insert(particles[i].position, 1.f);
		}

		//traverse about 10x longer than construct
		for (int i = 0; i < amount; i++) {
			particles[i].new_acceleration = Qtree.calc_forces_fast(particles[i].position, 1.f);
		}
	}


	// helper fuction for multithreading
	void traverse_multi(int n, int thread_nr) {
		for (int i = n * thread_nr; i < (thread_nr + 1) * n; i++) {
			particles[i].new_acceleration = Qtree.calc_forces_fast(particles[i].position, 1.f);
		}
	}

	// barnes hut multithreading
	void barnes_hut_multi() {

		for (int i = 0; i < amount; i++) {
			Qtree.insert(particles[i].position, 1.f);
		}

		int partition = amount / 4;

		std::thread t0(&Particlesystem::traverse_multi, this, partition, 0);
		std::thread t1(&Particlesystem::traverse_multi, this, partition, 1);
		std::thread t2(&Particlesystem::traverse_multi, this, partition, 2);
		std::thread t3(&Particlesystem::traverse_multi, this, partition, 3);

		t0.join();
		t1.join();
		t2.join();
		t3.join();

	}

	// loop for naive force calculation approach, collision possible, unused
	void loop_particles() {

		for (int i = 0; i < amount; i++) {

			Particle& p1 = particles[i];

			for (int j = i + 1; j < amount; j++) {

				Particle& p2 = particles[j];
				
				if (gravity_on) {
					calc_acceleration(p1, p2);
				}

				else if (collision_on) {
					resolve_collision(p1, p2);
				}
				else {
					calc_acceleration(p1, p2);
					resolve_collision(p1, p2);
				}
				}
			}
		}

	// naive n^n calculation of the forces between the particles
	void calc_acceleration_brute() {
		
		for (int i = 0; i < amount; i++) {

			glm::vec3 vector(0.f);
			glm::vec3 normal_vector(0.f);
			float distance = 0;
			float acceleration_scalar_1 = 0;
			float acceleration_scalar_2 = 0;
			//glm::vec3 accelerations(0.f);

			for (int j = i; j < amount; j++) {
				
				if (particles[i].position != particles[j].position) {
					vector = particles[j].position - particles[i].position;
					normal_vector = glm::normalize(vector);
					distance = glm::length(vector);

					acceleration_scalar_1 = gravitational_constant * particles[j].radius / ((distance * distance) + 0.001);
					acceleration_scalar_2 = gravitational_constant * particles[i].radius / ((distance * distance) + 0.001);

					particles[i].new_acceleration += acceleration_scalar_1 * normal_vector;
					particles[j].new_acceleration -= acceleration_scalar_2 * normal_vector;
				}
			}
			//particles[i].new_acceleration = accelerations;
		}
	}

	// unused
	void collision_check() {

		float distance = 0;
		float dot1 = 0;
		float dot2 = 0;

		glm::vec3 p2p1 = glm::vec3(0.f);
		glm::vec3 p1p2 = glm::vec3(0.f);

		float mass_factor1 = 0;
		float mass_factor2 = 0;

		float distance_sq = 0;
		
		for (int i = 0; i < amount; i++) {

			Particle& p1 = particles[i];

			for (int j = i + 1; j < amount; j++) {
				
				Particle& p2 = particles[j];
				distance = glm::length(p2.position - p1.position);
				
				if (distance < (p2.radius + p1.radius)) {
					
					dot1 = glm::dot(p1.velocity - p2.velocity, p1.position - p2.position);
					dot2 = glm::dot(p2.velocity - p1.velocity, p2.position - p1.position);

					distance_sq = distance * distance;

					p1p2 = p1.position - p2.position;
					p2p1 = p2.position - p1.position;

					mass_factor1 = 2 * p2.radius / (p1.radius + p2.radius);
					mass_factor2 = 2 * p1.radius / (p1.radius + p2.radius);

					p1.velocity = p1.velocity - mass_factor1 * (dot1 / distance_sq) * p1p2;
					p2.velocity = p2.velocity - mass_factor2 * (dot2 / distance_sq) * p2p1;
				}
			}
		}
	}

	// unused
	void resolve_collision(Particle& p1, Particle& p2) {
		float distance = glm::length(p2.position - p1.position);

		if (distance < (p2.radius + p1.radius)) {

			float dot1 = glm::dot(p1.velocity - p2.velocity, p1.position - p2.position);
			float dot2 = glm::dot(p2.velocity - p1.velocity, p2.position - p1.position);

			float distance_sq = distance * distance;

			glm::vec3 p1p2 = p1.position - p2.position;
			glm::vec3 p2p1 = p2.position - p1.position;

			float mass_factor1 = 2 * p2.radius / (p1.radius + p2.radius);
			float mass_factor2 = 2 * p1.radius / (p1.radius + p2.radius);

			p1.velocity = p1.velocity - mass_factor1 * (dot1 / distance_sq) * p1p2;
			p2.velocity = p2.velocity - mass_factor2 * (dot2 / distance_sq) * p2p1;
		}
	}

	// unused, helper for previous naive forces and collision simulation
	void calc_acceleration(Particle& p1, Particle& p2) {

		if (p1.position != p2.position) {
			glm::vec3 vector = p2.position - p1.position;
			glm::vec3 normaL_vector = glm::normalize(vector);
			float distance = glm::length(vector);

			float acc_scalar1 = gravitational_constant * p2.radius / (distance * distance + 0.1f);
			float acc_scalar2 = gravitational_constant * p1.radius / (distance * distance + 0.1f);

			p1.new_acceleration += acc_scalar1 * normaL_vector;
			p2.new_acceleration -= acc_scalar2 * normaL_vector;
		}
	}
};

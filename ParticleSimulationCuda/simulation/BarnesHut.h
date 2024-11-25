#pragma once
#include <vector>
#include <glm/glm.hpp>


/*contains the bounding box of a node
	Quadrant indices:
	3 | 0
	--|--
	2 | 1
*/
struct Quad {
	glm::vec3 center;
	float size;

	Quad() : center(), size(0.f) {};

	Quad(glm::vec3 c, float s) : center(c), size(s) {};

	glm::vec3 new_quadrant(int q) {

		glm::vec3 quad_center;

		switch (q) {
		case 0:
			quad_center = glm::vec3(center.x + size / 4, center.y + size / 4, 0.f);
			return quad_center;
		case 1:
			quad_center = glm::vec3(center.x + size / 4, center.y - size / 4, 0.f);
			return quad_center;
		case 2:
			quad_center = glm::vec3(center.x - size / 4, center.y - size / 4, 0.f);
			return quad_center;
		case 3:
			quad_center = glm::vec3(center.x - size / 4, center.y + size / 4, 0.f);
			return quad_center;
		}

	}

	int find_quadrant(glm::vec3 v) {

		glm::vec3 rel_pos = v - center;

		//check if point is in bounds of overall box
		if (v.x > center.x + size / 2 || v.x < center.x - size / 2) {
			return NULL;
		}

		if (v.y > center.y + size / 2 || v.y < center.y - size / 2) {
			return NULL;
		}

		//find the correct quadrant
		if (rel_pos.x > 0) {
			if (rel_pos.y > 0) {
				//top_right
				return 0;
			}
			else {
				//bottom right
				return 1;
			}
		}
		else {
			if (rel_pos.y > 0) {
				//top_left
				return 3;
			}
			else {
				//bot_left
				return 2;
			}
		}
	}

};

//contains information about the indices of its children
struct Node {
	int children; //index of the children in the nodes array
	int parent;
	bool is_leaf; //true = leaf, false = branch

	glm::vec3 center_mass;
	float mass;
	Quad quad;

	Node() : children(0), mass(0.f), quad(), is_leaf(true), center_mass(0.f), parent(0) {};

	//check if the body is sufficently far away from the nodes center of mass to split
	bool check_criterion(glm::vec3 pos_body, float theta) {
		glm::vec3 delta_center_mass = glm::vec3((center_mass.x - pos_body.x), (center_mass.y, pos_body.y), 0.f);

		float d = delta_center_mass.x * delta_center_mass.x + delta_center_mass.y * delta_center_mass.y;
		float s_sq = quad.size * quad.size;

		if (s_sq / d < theta) {
			return true;
		}
		else {
			return false;
		}
	}
};

struct Quadtree {

	const int root = 0;

	std::vector<Node> nodes;
	std::vector<int> parents;

	std::vector <bool> blocked_parents; //not needed for the final use

	float gravitational_constant;
	float theta;
	float min_Quad_size; //sets the smallest size of a quad, not implemented

	Quadtree() : nodes(), parents(), gravitational_constant(0.00001f), theta(0.9f), min_Quad_size(0.01f) { init_root_node(); };

	void init_root_node() {
		Node root_node = Node();
		root_node.quad.center = glm::vec3(0.f);
		root_node.quad.size = 100.f;
		nodes.push_back(root_node);
	}

	// recursively inserts a point into the quadtree, either expands it or adds it to node
	void insert(glm::vec3 &pos, float mass) {

		int current_node = root;

		//navigates down the existing internal / non leaf nodes and updates them until a leaf node is reached
		while (!nodes[current_node].is_leaf) {
			nodes[current_node].mass += mass;
			nodes[current_node].center_mass.x + pos.x;
			nodes[current_node].center_mass.y + pos.y;

			//find the index of the child node representing the right quadrant for the point
			int quadrant = nodes[current_node].quad.find_quadrant(pos);

			current_node = nodes[current_node].children + quadrant;
		}


		while (true) {

			// if the leaf node is empty, the point is added to it
			if (nodes[current_node].mass == 0) {
				nodes[current_node].mass += mass;
				nodes[current_node].center_mass += pos;
				return;
			}

			//no free node in existing tree, tree is further subdivided, current node gets 4 children, becomes internal node
			nodes[current_node].children = nodes.size();
			nodes[current_node].is_leaf = false;

			//child nodes are created for each quadrant
			for (int i = 0; i < 4; i++) {

				Node child = Node();
				child.parent = current_node;
				child.quad = Quad(nodes[current_node].quad.new_quadrant(i), nodes[current_node].quad.size / 2);
				nodes.push_back(child);
			}

			//previously to current node attached point is passed down to the appropiate child node, since current node is not a leaf node anymore 

			Node& pass_child = nodes[nodes[current_node].children + nodes[current_node].quad.find_quadrant(nodes[current_node].center_mass)];
			pass_child.center_mass.x = nodes[current_node].center_mass.x;
			pass_child.center_mass.y = nodes[current_node].center_mass.y;
			pass_child.mass = nodes[current_node].mass;

			//update center of mass and mass, of parent node
			nodes[current_node].mass += mass;
			nodes[current_node].center_mass.x += pos.x;
			nodes[current_node].center_mass.y += pos.y;

			//the child node with the correct quadrant becomes the new current node
			current_node = nodes[current_node].quad.find_quadrant(pos) + nodes[current_node].children;
		}

	}


	//horrible N^K way to traverse the tree and calculate forces, not used
	glm::vec3 calc_forces(glm::vec3 &pos, float mass) {

		float distance = 0.f;
		glm::vec3 direction_vector(0.f);
		glm::vec3 acceleration(0.f);

		//if a node is used for an approximation in relation to a body, the value of the array at node index becomes true, so that its children arent used further
		//this does not seem to be a good approach, as it still requires bodies * nodes comparisions

		blocked_parents = std::vector<bool>(nodes.size(), false);

		for (int current_node = 0; current_node < nodes.size(); current_node++) {
			//check if node contains body / bodies. 0.9f because floating point issues maybe??
			//And check if Node has been approximated by parent node
			if (!blocked_parents[current_node] && nodes[current_node].mass > 0.9f) {

				direction_vector = (nodes[current_node].center_mass / nodes[current_node].mass) - pos; //Vector pointing from Body to Nodes center of mass
				distance = glm::length(direction_vector);

				if (distance == 0) {
					continue;
				}
				//if node is leaf -> contains just one body
				if (nodes[current_node].is_leaf) {
					acceleration += calc_acceleration(1.f, nodes[current_node].mass, distance, direction_vector);
				}
				//if it isnt a leaf, contains multiple bodies, check if node is sufficently far away from body, to approximate the force
				else if (nodes[current_node].quad.size / distance < theta) {
					acceleration += calc_acceleration(1.f, nodes[current_node].mass, distance, direction_vector);

					//no children of that node will be considered
					blocked_parents[current_node] = true;
				}
			}

		}
		return acceleration;

	}

	//Nice optimal n * log(n) way to traverse
	//tree is recursively traversed until the leaf nodes are reached or the node is sufficently far away from the point to approximate
	void traverse_tree(int current_node, glm::vec3 &pos, glm::vec3 &acceleration) {

		// goes into the indices of the nodes children
		for (int i = 0; i < 4; i++) {
			
			int child_id = nodes[current_node].children + i;

			//the tree is not pruned, so check if the node is empty
			if (nodes[child_id].mass == 0) {
				continue;
			}

			// calculate distance direction vector to get the distance and to be used for the force calculation
			glm::vec3 direction_vector = (nodes[child_id].center_mass / nodes[child_id].mass) - pos; //Vector pointing from Body to Nodes center of mass
			float distance = glm::length(direction_vector);

			//check against nodes center of mass instead? Avoids prior direction vector calculation
			if (distance == 0) {
				continue;
			}
			
			//if a leaf node is reached, the force / acceleration is calculated
			if (nodes[child_id].is_leaf) {
				acceleration += calc_acceleration(1.f, nodes[child_id].mass, distance, direction_vector);
				continue;
			}

			//if the node is sufficently far away, treat the node as single body to approximate the force
			if ((nodes[child_id].quad.size / distance) < theta) {
				acceleration += calc_acceleration(1.f, nodes[child_id].mass, distance, direction_vector);
				continue;
			}
			else {
				// THIS BREAKS THE RECURSION
				//current_node = child_id breaks the recursion loop when traversing from child to parent. As the loop for the parent has now the ID of the child as its current node
				//current_node = child_id;
				
				traverse_tree(child_id, pos, acceleration);
			}
		}
	}

	glm::vec3 calc_forces_fast(glm::vec3& pos, float mass) {
		glm::vec3 acceleration(0.f);
		int current_node = 0;

		traverse_tree(current_node, pos, acceleration);

		return acceleration;
	}


	//gravitational constant in Quadtree is nonsense, should be in ParticleSystem
	glm::vec3 calc_acceleration(float mb, float mn, float d, glm::vec3 &d_v) {

		float acceleration_scalar = gravitational_constant * (mn * mn) / ((d * d) + 0.01);

		glm::vec3 norm = glm::normalize(d_v);

		return acceleration_scalar * norm;
	}

};
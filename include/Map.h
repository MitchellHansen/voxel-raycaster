#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <iostream>
#include <list>
#include <random>
#include <iostream>
#include <functional>
#include <cmath>
#include "util.hpp"
#include <deque>
#include <unordered_map>
#include <bitset>

#define _USE_MATH_DEFINES
#include <math.h>

#define CHUNK_DIM 32
#define OCT_DIM 64

struct KeyHasher {
	std::size_t operator()(const sf::Vector3i& k) const {

		return ((std::hash<int>()(k.x)
			^ (std::hash<int>()(k.y) << 1)) >> 1)
			^ (std::hash<int>()(k.z) << 1);
	}
};

struct Chunk {
	Chunk(int type) { voxel_data = new int[CHUNK_DIM * CHUNK_DIM * CHUNK_DIM]; set(type); };
	Chunk() { };
	void set(int type);
	~Chunk() { voxel_data = nullptr; };
	int* voxel_data;
};

class Octree {
	
public:
	Octree() {
		dat = new uint64_t[(int)pow(2, 15)];
		for (int i = 0; i < (int)pow(2, 15); i++) {
			dat[i] = 0;
		}
	};

	~Octree() {};

	uint64_t *dat;
	uint64_t stack_pos = 0x8000;
	uint64_t global_pos = 0;
	
	uint64_t copy_to_stack(std::vector<uint64_t> children) {
		
		// Check for the 15 bit boundry		
		if (stack_pos - children.size() > stack_pos) {
			global_pos = stack_pos;
			stack_pos = 0x8000;
		}
		else {
			stack_pos -= children.size();
		}

		// Check for the far bit

		memcpy(&dat[stack_pos + global_pos], children.data(), children.size() * sizeof(uint64_t));
		
		// Return the bitmask encoding the index of that value
		// If we tripped the far bit, allocate a far index to the stack and place
		// it one above preferably.
		// And then shift the far bit to 1

		// If not, shift the index to its correct place
		return stack_pos;
	};

	

};


class Map {
public: 

	Map(sf::Vector3i dim);
	void generate_octree();

	void load_unload(sf::Vector3i world_position);
	void load_single(sf::Vector3i world_position);

	sf::Vector3i getDimensions();
	char *list;
	//sf::Vector3i dimensions;

	void setVoxel(sf::Vector3i position, int val);

	void moveLight(sf::Vector2f in);
	sf::Vector3f global_light;

	Octree a;

protected:

private:

	// DEBUG
	int counter = 0;
	std::stringstream ss;

	// !DEBUG


	uint64_t generate_children(sf::Vector3i pos, int dim);


	char getVoxel(sf::Vector3i pos);
	char* voxel_data = new char[OCT_DIM * OCT_DIM * OCT_DIM];

	std::unordered_map<sf::Vector3i, Chunk, KeyHasher> chunk_map;

	double* height_map;

	// 2^k
	int chunk_radius = 6;

	sf::Vector3i world_to_chunk(sf::Vector3i world_coords) {
		return sf::Vector3i(
			world_coords.x / CHUNK_DIM + 1,
			world_coords.y / CHUNK_DIM + 1,
			world_coords.z / CHUNK_DIM + 1
			);
	}
};



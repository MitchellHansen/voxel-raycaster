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

protected:

private:


	int64_t generate_children(sf::Vector3i pos, int dim);


	int64_t block[1024];
	int stack_position = 0;
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



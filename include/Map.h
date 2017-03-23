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
#include <deque>
#include <unordered_map>
#include <bitset>
#include <cstring>
#include <queue>
#include "util.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#define CHUNK_DIM 32
#define OCT_DIM 32

struct XYZHasher {
	std::size_t operator()(const sf::Vector3i& k) const {
		return ((std::hash<int>()(k.x)
			^ (std::hash<int>()(k.y) << 1)) >> 1)
			^ (std::hash<int>()(k.z) << 1);
	}
};

class Octree {
public:
	Octree();
	~Octree() {};

	uint64_t *blob = new uint64_t[100000];

	uint64_t stack_pos = 0x8000;
	uint64_t global_pos = 0;
	
	uint64_t copy_to_stack(std::vector<uint64_t> children);

	// With a position and the head of the stack. Traverse down the voxel hierarchy to find
	// the IDX and stack position of the highest resolution (maybe set resolution?) oct
	bool get_voxel(sf::Vector3i position);

	void print_block(int block_pos);

private:

	// (X, Y, Z) mask for the idx
	const uint8_t idx_set_x_mask = 0x1;
	const uint8_t idx_set_y_mask = 0x2;
	const uint8_t idx_set_z_mask = 0x4;

	// Mask for 
	const uint8_t mask_8[8] = {
		0x1,  0x2,  0x4,  0x8,
		0x10, 0x20, 0x40, 0x80
	};

	const uint8_t count_mask_8[8]{
		0x1,  0x3,  0x7,  0xF,
		0x1F, 0x3F, 0x7F, 0xFF
	};

	const uint64_t child_pointer_mask = 0x0000000000007fff;
	const uint64_t far_bit_mask = 0x8000;
	const uint64_t valid_mask = 0xFF0000;
	const uint64_t leaf_mask = 0xFF000000;
	const uint64_t contour_pointer_mask = 0xFFFFFF00000000;
	const uint64_t contour_mask = 0xFF00000000000000;

};


class Map {
public: 

	Map(sf::Vector3i position);

	void generate_octree();

	void setVoxel(sf::Vector3i position, int val);

	char getVoxelFromOctree(sf::Vector3i position);

	bool getVoxel(sf::Vector3i pos);
	Octree a;

	void test_map();

private:

	// ======= DEBUG ===========
	int counter = 0;
	std::stringstream output_stream;
	// =========================


	uint64_t generate_children(sf::Vector3i pos, int dim);

	char* voxel_data = new char[OCT_DIM * OCT_DIM * OCT_DIM];

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



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
#include <cstring>
#include <queue>

#define _USE_MATH_DEFINES
#include <math.h>

#define CHUNK_DIM 32
#define OCT_DIM 8

struct XYZHasher {
	std::size_t operator()(const sf::Vector3i& k) const {
		return ((std::hash<int>()(k.x)
			^ (std::hash<int>()(k.y) << 1)) >> 1)
			^ (std::hash<int>()(k.z) << 1);
	}
};





class Octree {
public:
	Octree() {

		// initialize the first stack block
		block_stack.push_back(new uint64_t[0x8000]);
		for (int i = 0; i < 0x8000; i++) {
			block_stack.back()[i] = 0;
		}


	};

	~Octree() {};

	std::list<uint64_t*> block_stack;
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

		memcpy(&block_stack.front()[stack_pos + global_pos], children.data(), children.size() * sizeof(uint64_t));
		
		// Return the bitmask encoding the index of that value
		// If we tripped the far bit, allocate a far index to the stack and place
		// it one above preferably.
		// And then shift the far bit to 1

		// If not, shift the index to its correct place
		return stack_pos;
	};


	int get_idx(sf::Vector3i voxel_pos) {
	
		return 1;
		
	}

	// This might need to be a recursive function. But it needs to be easily ported to
	// OpenCL C. Might spend some time thinking about how to do this in a linear algorithm
	bool get_voxel(sf::Vector3i position) {

		std::queue<uint64_t> parent_stack;

		uint64_t head = block_stack.front()[stack_pos];

		parent_stack.push(head);

		uint64_t index = head & child_pointer_mask;

		int dimension = OCT_DIM;
		sf::Vector3i quad_position(0, 0, 0);

		while (dimension > 1) {
			
			sf::Vector3i p;
			if (position.x >= (dimension / 2) + quad_position.x)
				quad_position.x += (dimension / 2);
			if (position.y >= (dimension / 2) + quad_position.y)
				quad_position.y += (dimension / 2);
			if (position.z >= (dimension / 2) + quad_position.z)
				quad_position.z += (dimension / 2);

			dimension /= 2;

		}

		uint64_t child1 = block_stack.front()[index];
        uint64_t child2 = block_stack.front()[index+1];

        std::bitset<64> t(index);
        auto val = t.count();

		return true;
	}


	void print_block(int block_pos) {
		std::stringstream sss;
		for (int i = 0; i < (int)pow(2, 15); i++) {
			PrettyPrintUINT64(block_stack.front()[i], &sss);
			sss << "\n";
		}
		DumpLog(&sss, "raw_data.txt");
	}

private:

	const uint64_t child_pointer_mask = 0x0000000000007fff;
	const uint64_t far_bit_mask = 0x8000;
	const uint64_t valid_mask = 0xFF0000;
	const uint64_t leaf_mask = 0xFF000000;
	const uint64_t contour_pointer_mask = 0xFFFFFF00000000;
	const uint64_t contour_mask = 0xFF00000000000000;

	
	

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

	char getVoxelFromOctree(sf::Vector3i position);

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

	//std::unordered_map<sf::Vector3i, Chunk, XYZHasher> chunk_map;

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



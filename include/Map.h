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
#include "util.cpp"
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

	// (X, Y, Z) mask for the idx
	uint8_t idx_set_x_mask = 0x1;
	uint8_t idx_set_y_mask = 0x2;
	uint8_t idx_set_z_mask = 0x4;

	uint8_t mask_8[8] = {
		0x0, 0x1, 0x2, 0x3,
		0x4, 0x5, 0x6, 0x7
	};

	uint8_t count_mask_8[8]{
		0x1,  0x3,  0x7,  0xF,
		0x1F, 0x3F, 0x7F, 0xFF
	};

	// With a position and the head of the stack. Traverse down the voxel hierarchy to find
	// the IDX and stack position of the highest resolution (maybe set resolution?) oct
	bool get_voxel(sf::Vector3i position) {

		// Init the parent stack and push the head node
		//std::queue<uint64_t> parent_stack;

		int parent_stack_position = 0;
		uint64_t parent_stack[32] = {0};
		
		uint64_t head = block_stack.front()[stack_pos];
		parent_stack[parent_stack_position] = head;
		

		// Get the index of the first child of the head node
		uint64_t index = head & child_pointer_mask;

		uint8_t scale = 0;
		uint8_t idx_stack[32] = {0};

		// Init the idx stack
		std::vector<std::bitset<3>> scale_stack(log2(OCT_DIM));

		// Set our initial dimension and the position we use to keep track what oct were in
		int dimension = OCT_DIM;
		sf::Vector3i quad_position(0, 0, 0);

		while (dimension > 1) {
			
			// So we can be a little bit tricky here and increment our
			// array index that holds our masks as we build the idx. 
			// Adding 1 for X, 2 for Y, and 4 for Z
			int mask_index = 0;


			// Do the logic steps to find which sub oct we step down into
			if (position.x >= (dimension / 2) + quad_position.x) {

				// Set our voxel position to the (0,0) of the correct oct
				quad_position.x += (dimension / 2);

				// increment the mask index and mentioned above
				mask_index += 1;

				// Set the idx to represent the move
				idx_stack[scale] |= idx_set_x_mask;

				// Debug
				scale_stack.at(log2(OCT_DIM) - log2(dimension)).set(0);
				
			}
			if (position.y >= (dimension / 2) + quad_position.y) {

				quad_position.y |= (dimension / 2);

				mask_index += 2;

				idx_stack[scale] ^= idx_set_y_mask;
				scale_stack.at(log2(OCT_DIM) - log2(dimension)).set(1);
			}
			if (position.z >= (dimension / 2) + quad_position.z) {

				quad_position.z += (dimension / 2);

				mask_index += 4;

				idx_stack[scale] |= idx_set_z_mask;
				scale_stack.at(log2(OCT_DIM) - log2(dimension)).set(2);
			}

			// Check to see if we are on a valid oct
			if ((head >> 16) & mask_8[mask_index]) {
				
				// Check to see if it is a leaf
				if ((head >> 24) & mask_8[mask_index]) {

					// If it is, then we cannot traverse further as CP's won't have been generated
					break;
				}

				// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
				scale++;
				dimension /= 2;

				// We also need to traverse to the correct child pointer
				
				// Count the number of non-leaf octs that come before and add it to the current parent stack position
				int count = count_bits((uint8_t)(head >> 24) ^ count_mask_8[mask_index]);
				int index = (parent_stack[parent_stack_position] & child_pointer_mask) + count;
				
				// Increment the parent stack position and put the new oct node as the parent
				parent_stack_position++;
				parent_stack[parent_stack_position] = block_stack.front()[index];
			
			} else {
				// If the oct was not valid, then no CP's exists any further


				// It appears that the traversal is now working but I need
				// to focus on how to now take care of the end condition.
				// Currently it adds the last parent on the second to lowest
				// oct CP. Not sure if thats correct
				break;
			}
		}


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



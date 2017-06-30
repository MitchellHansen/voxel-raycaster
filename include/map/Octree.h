#pragma once
#include <SFML/System/Vector3.hpp>
#include <vector>
#include "util.hpp"
#include <tuple>

#define OCT_DIM 32

struct oct_state {

	int parent_stack_position = 0;
	uint64_t parent_stack[32] = { 0 };

	uint8_t scale = 0;
	uint8_t idx_stack[32] = { 0 };

	uint64_t current_descriptor;

};


class Octree {
public:

	static const int buffer_size = 100000;


	Octree();
	~Octree() {};

	void Generate(char* data, sf::Vector3i dimensions);
	void Load(std::string octree_file_name);
	
	uint64_t *trunk_buffer				= new uint64_t[buffer_size]{0};
	uint64_t trunk_buffer_position		= buffer_size;

	uint64_t *descriptor_buffer			= new uint64_t[buffer_size]{0};
	uint64_t descriptor_buffer_position = buffer_size;

	uint32_t *attachment_lookup			= new uint32_t[buffer_size]{0};
	uint64_t attachment_lookup_position = buffer_size;

	uint64_t *attachment_buffer			= new uint64_t[buffer_size]{0};
	uint64_t attachment_buffer_position = buffer_size;

	unsigned int trunk_cutoff = 3;
	uint64_t root_index = 0;

	int page_header_counter = 0x8000;
	uint64_t current_info_section_position = buffer_size - 50;

	uint64_t stack_pos = 0x8000;
	uint64_t global_pos = buffer_size - 50;

	uint64_t copy_to_stack(std::vector<uint64_t> children, unsigned int voxel_scale);

	// With a position and the head of the stack. Traverse down the voxel hierarchy to find
	// the IDX and stack position of the highest resolution (maybe set resolution?) oct
	bool get_voxel(sf::Vector3i position);

	void print_block(int block_pos);

private:

	std::tuple<uint64_t, uint64_t> GenerationRecursion(
		char* data,					// raw octree data
		sf::Vector3i dimensions,	// dimensions of the raw data
		sf::Vector3i pos,			// position of this generation node
		unsigned int voxel_scale	// the voxel scale of this node
	); 

	static char get1DIndexedVoxel(char* data, sf::Vector3i dimensions, sf::Vector3i position);

	std::vector<uint64_t> anchor_stack;
	unsigned int octree_voxel_dimension = 32;

	// (X, Y, Z) mask for the idx
	const uint8_t idx_set_x_mask = 0x1;
	const uint8_t idx_set_y_mask = 0x2;
	const uint8_t idx_set_z_mask = 0x4;

	// Mask for checking if valid or leaf
	const uint8_t mask_8[8] = {
		0x1,  0x2,  0x4,  0x8,
		0x10, 0x20, 0x40, 0x80
	};

	// Mask for counting the previous valid bits
	const uint8_t count_mask_8[8]{
		0x1,  0x3,  0x7,  0xF,
		0x1F, 0x3F, 0x7F, 0xFF
	};


	// uint64_t manipulation masks
	const uint64_t child_pointer_mask = 0x0000000000007fff;
	const uint64_t far_bit_mask = 0x8000;
	const uint64_t valid_mask = 0xFF0000;
	const uint64_t leaf_mask = 0xFF000000;
	const uint64_t contour_pointer_mask = 0xFFFFFF00000000;
	const uint64_t contour_mask = 0xFF00000000000000;


	// ======= DEBUG ===========
	int counter = 0;
	std::stringstream output_stream;
	// =========================
};

#pragma once
#include <SFML/System/Vector3.hpp>
#include <vector>
#include "util.hpp"
#include <tuple>

struct OctState {

	int parent_stack_position = 0;
	uint64_t parent_stack[32] = { 0 };

	uint8_t scale = 0;
	uint8_t idx_stack[32] = { 0 };

	uint64_t current_descriptor;

	sf::Vector3i oct_pos;

	// ====== DEBUG =======
	char found = 1;
};


class Octree {
public:

	static const int buffer_size = 300000;

	Octree();
	~Octree() {};

	// Generate an octree from 3D indexed array of char data
	void Generate(char* data, sf::Vector3i dimensions);

	// TODO: Load the octree from a serialized or whatever file
	void Load(std::string octree_file_name);
	
	// I think the best way to transfer all of the data to the GPU. Each buffer will contain a set of blocks
	// except for the trunk buffer. The paper indicates that the cutoff point for the trunk can vary,
	// but since I'm going to do seperate buffers, I'm going to set a hard cutoff for the trunk so we
	// know when to switch buffers

	uint64_t *descriptor_buffer;
	uint64_t descriptor_buffer_position = buffer_size;

	uint32_t *attachment_lookup;
	uint64_t attachment_lookup_position = buffer_size;

	uint64_t *attachment_buffer;
	uint64_t attachment_buffer_position = buffer_size;

	unsigned int trunk_cutoff = 3;
	uint64_t root_index = 0;

	int page_header_counter = 0x8000;

	// Cheat and underflow to get the position
	uint64_t current_info_section_position = ((uint64_t)0)-1;
	

	uint64_t stack_pos = 0x8000;
	uint64_t global_pos = buffer_size - 50;
	
	// With a position and the head of the stack. Traverse down the voxel hierarchy to find
	// the IDX and stack position of the highest resolution (maybe set resolution?) oct
	OctState GetVoxel(sf::Vector3i position);

	void print_block(int block_pos);

    bool Validate(char* data, sf::Vector3i dimensions); 

	unsigned int getDimensions();

	// (X, Y, Z) mask for the idx
	static const uint8_t idx_set_x_mask = 0x1;
	static const uint8_t idx_set_y_mask = 0x2;
	static const uint8_t idx_set_z_mask = 0x4;

	// Mask for checking if valid or leaf
	static const uint8_t mask_8[8];

	// Mask for counting the previous valid bits
	static const uint8_t count_mask_8[8];


	// uint64_t manipulation masks
	static const uint64_t child_pointer_mask = 0x0000000000007fff;
	static const uint64_t far_bit_mask = 0x8000;
	static const uint64_t valid_mask = 0xFF0000;
	static const uint64_t leaf_mask = 0xFF000000;
	static const uint64_t contour_pointer_mask = 0xFFFFFF00000000;
	static const uint64_t contour_mask = 0xFF00000000000000;

	std::vector<std::tuple<sf::Vector3i, char>> Octree::CastRayOctree(
		sf::Vector2f cam_dir,
		sf::Vector3f cam_pos
	);

private:

	unsigned int oct_dimensions = 1;

	std::tuple<uint64_t, uint64_t> GenerationRecursion(
		char* data,					// raw octree data
		sf::Vector3i dimensions,	// dimensions of the raw data
		sf::Vector3i pos,			// position of this generation node
		unsigned int voxel_scale	// the voxel scale of this node
	); 
	
	char get1DIndexedVoxel(char* data, sf::Vector3i dimensions, sf::Vector3i position);

	std::vector<uint64_t> anchor_stack;
	unsigned int octree_voxel_dimension = 32;




	// ======= DEBUG ===========
	int counter = 0;
	std::stringstream output_stream;
	// =========================
};

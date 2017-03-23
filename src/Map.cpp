#include "Map.h"

void SetBit(int position, char* c) {
	*c |= (uint64_t)1 << position;
}

void FlipBit(int position, char* c) {
	*c ^= (uint64_t)1 << position;
}

int GetBit(int position, char* c) {
	return (*c >> position) & (uint64_t)1;
}

void SetBit(int position, uint64_t* c) {
	*c |= (uint64_t)1 << position;
}

void FlipBit(int position, uint64_t* c) {
	*c ^= (uint64_t)1 << position;
}

int GetBit(int position, uint64_t* c) {
	return (*c >> position) & (uint64_t)1;
}

bool CheckLeafSign(const uint64_t descriptor) {

	uint64_t valid_mask = 0xFF0000;

	// Return true if all 1's, false if contiguous 0's
	if ((descriptor & valid_mask) == valid_mask) {
		return true;
	}
	if ((descriptor & valid_mask) == 0) {
		return false;
	}

	// Error out, something funky
	abort();
}

bool CheckContiguousValid(const uint64_t c) {
	uint64_t bitmask = 0xFF0000;
	return (c & bitmask) == bitmask;
}



bool IsLeaf(const uint64_t descriptor) {

	uint64_t leaf_mask = 0xFF000000;
	uint64_t valid_mask = 0xFF0000;

	// Check for contiguous valid values of either 0's or 1's
	if (((descriptor & valid_mask) == valid_mask) || ((descriptor & valid_mask) == 0)) {
		
		// Check for a full leaf mask
		// Only if valid and leaf are contiguous, then it's a leaf
		if ((descriptor & leaf_mask) == leaf_mask)
			return true;
		else
			return false;
	}
	else
		return false;
}

Map::Map(sf::Vector3i position) {

	srand(time(NULL));

	for (int i = 0; i < OCT_DIM * OCT_DIM * OCT_DIM; i++) {
		if (rand() % 25 < 2)
			voxel_data[i] = 1;
		else
			voxel_data[i] = 0;
	}

}

uint64_t Map::generate_children(sf::Vector3i pos, int voxel_scale) {


	// The 8 subvoxel coords starting from the 1th direction, the direction of the origin of the 3d grid
	// XY, Z++, XY
	std::vector<sf::Vector3i> v = { 
		sf::Vector3i(pos.x      , pos.y      , pos.z),
		sf::Vector3i(pos.x + voxel_scale, pos.y      , pos.z),
		sf::Vector3i(pos.x      , pos.y + voxel_scale, pos.z),
		sf::Vector3i(pos.x + voxel_scale, pos.y + voxel_scale, pos.z),
		sf::Vector3i(pos.x      , pos.y      , pos.z + voxel_scale),
		sf::Vector3i(pos.x + voxel_scale, pos.y      , pos.z + voxel_scale),
		sf::Vector3i(pos.x      , pos.y + voxel_scale, pos.z + voxel_scale),
		sf::Vector3i(pos.x + voxel_scale, pos.y + voxel_scale, pos.z + voxel_scale) 
	};

	// If we hit the 1th voxel scale then we need to query the 3D grid
	// and get the voxel at that position. I assume in the future when I
	// want to do chunking / loading of raw data I can edit the voxel access
	if (voxel_scale == 1) {

		// 
		uint64_t child_descriptor = 0;

		// Setting the individual valid mask bits
		// These don't bound check, should they?
		for (int i = 0; i < v.size(); i++) {
			if (getVoxel(v.at(i)))
				SetBit(i + 16, &child_descriptor);
		}

		// We are querying leafs, so we need to fill the leaf mask
		child_descriptor |= 0xFF000000;

		// This is where contours 
		// The CP will be left blank, contours will be added maybe
		return child_descriptor;

	}

	// Init a blank child descriptor for this node
	uint64_t child_descriptor = 0;
	
	std::vector<uint64_t> descriptor_array;

	// Generate down the recursion, returning the descriptor of the current node
	for (int i = 0; i < v.size(); i++) {

		uint64_t child = 0;

		// Get the child descriptor from the i'th to 8th subvoxel
		child = generate_children(v.at(i), voxel_scale / 2);

		// =========== Debug ===========
		PrettyPrintUINT64(child, &output_stream);
		output_stream << "    " << voxel_scale << "    " << counter++ << std::endl;
		// =============================

		// If the child is a leaf (contiguous) of non-valid values
		if (IsLeaf(child) && !CheckLeafSign(child)) {
			// Leave the valid mask 0, set leaf mask to 1
			SetBit(i + 16 + 8, &child_descriptor);
		}

		// If the child is valid and not a leaf
		else {

			// Set the valid mask, and add it to the descriptor array
			SetBit(i + 16, &child_descriptor);
			descriptor_array.push_back(child);
		}
	}

	// Any free space between the child descriptors must be added here in order to 
	// interlace them and allow the memory handler to work correctly.

	// Copy the children to the stack and set the child_descriptors pointer to the correct value
	child_descriptor |= a.copy_to_stack(descriptor_array);

	// Free space may also be allocated here as well
	
	// Return the node up the stack
	return child_descriptor;
}

void Map::generate_octree() {

	// Launch the recursive generator at (0,0,0) as the first point
	// and the octree dimension as the initial block size
	uint64_t root_node = generate_children(sf::Vector3i(0, 0, 0), OCT_DIM/2);
	uint64_t tmp = 0;

	// ========= DEBUG ==============
	// PrettyPrintUINT64(root_node, &output_stream);
	// output_stream << "    " << OCT_DIM << "    " << counter++ << std::endl;
	// ==============================

	int position = a.copy_to_stack(std::vector<uint64_t>{root_node});

	// Dump the debug log
	// DumpLog(&output_stream, "raw_output.txt");

}

void Map::setVoxel(sf::Vector3i world_position, int val) {

}

char Map::getVoxelFromOctree(sf::Vector3i position)
{
	return a.get_voxel(position);
}

bool Map::getVoxel(sf::Vector3i pos){

	if (voxel_data[pos.x + OCT_DIM * (pos.y + OCT_DIM * pos.z)]) {
		return true;
	} else {
		return false;
	}
}

void Map::test_map() {

	std::cout << "Validating map..." << std::endl;

	for (int x = 0; x < OCT_DIM; x++) {
		for (int y = 0; y < OCT_DIM; y++) {
			for (int z = 0; z < OCT_DIM; z++) {

				sf::Vector3i pos(x, y, z);

				bool arr1 = getVoxel(pos);
				bool arr2 = getVoxelFromOctree(pos);

				if (arr1 != arr2) {
					std::cout << "X: " << pos.x << "Y: " << pos.y << "Z: " << pos.z << std::endl;
				}

			}
		}
	}

	std::cout << "Done" << std::endl;

	sf::Clock timer;
	
	timer.restart();
	
	for (int x = 0; x < OCT_DIM; x++) {
		for (int y = 0; y < OCT_DIM; y++) {
			for (int z = 0; z < OCT_DIM; z++) {

				sf::Vector3i pos(x, y, z);

				bool arr2 = getVoxelFromOctree(pos);
			}
		}
	}

	std::cout << "Octree linear xyz access : ";
	std::cout << timer.restart().asMicroseconds() << " microseconds" << std::endl;

	for (int x = 0; x < OCT_DIM; x++) {
		for (int y = 0; y < OCT_DIM; y++) {
			for (int z = 0; z < OCT_DIM; z++) {

				sf::Vector3i pos(x, y, z);

				bool arr1 = getVoxel(pos);
			}
		}
	}

	std::cout << "Array linear xyz access : ";
	std::cout << timer.restart().asMicroseconds() << " microseconds" << std::endl;

}

Octree::Octree() {
	
	// initialize the first stack block

	for (int i = 0; i < 0x8000; i++) {
		blob[i] = 0;
	}
}

uint64_t Octree::copy_to_stack(std::vector<uint64_t> children) {
	
	// Check for the 15 bit boundry		
	if (stack_pos - children.size() > stack_pos) {
		global_pos = stack_pos;
		stack_pos = 0x8000;
	}
	else {
		stack_pos -= children.size();
	}

	// Check for the far bit

	memcpy(&blob[stack_pos + global_pos], children.data(), children.size() * sizeof(uint64_t));

	// Return the bitmask encoding the index of that value
	// If we tripped the far bit, allocate a far index to the stack and place
	// it at the bottom of the child_descriptor node level array
	// And then shift the far bit to 1

	// If not, shift the index to its correct place
	return stack_pos;
}

bool Octree::get_voxel(sf::Vector3i position) {

	// Init the parent stack 
	int parent_stack_position = 0;
	uint64_t parent_stack[32] = { 0 };

	// and push the head node
	uint64_t head = blob[stack_pos];
	parent_stack[parent_stack_position] = head;

	// Get the index of the first child of the head node
	uint64_t index = head & child_pointer_mask;

	// Init the idx stack
	uint8_t scale = 0;
	uint8_t idx_stack[32] = { 0 };

	// Init the idx stack (DEBUG)
	//std::vector<std::bitset<3>> scale_stack(static_cast<uint64_t>(log2(OCT_DIM)));

	// Set our initial dimension and the position at the corner of the oct to keep track of our position
	int dimension = OCT_DIM;
	sf::Vector3i quad_position(0, 0, 0);

	// While we are not at the required resolution
	//		Traverse down by setting the valid/leaf mask to the subvoxel
	//		Check to see if it is valid
	//			Yes? 
	//				Check to see if it is a leaf
	//				No? Break
	//				Yes? Scale down to the next hierarchy, push the parent to the stack
	//		
	//			No?
	//				Break
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
			//scale_stack.at(static_cast<uint64_t>(log2(OCT_DIM) - log2(dimension))).set(0);

		}
		if (position.y >= (dimension / 2) + quad_position.y) {

			quad_position.y |= (dimension / 2);

			mask_index += 2;

			idx_stack[scale] ^= idx_set_y_mask;
			//scale_stack.at(static_cast<uint64_t>(log2(OCT_DIM) - log2(dimension))).set(1);
		}
		if (position.z >= (dimension / 2) + quad_position.z) {

			quad_position.z += (dimension / 2);

			mask_index += 4;

			idx_stack[scale] |= idx_set_z_mask;
			//scale_stack.at(static_cast<uint64_t>(log2(OCT_DIM) - log2(dimension))).set(2);
		}

		uint64_t out1 = (head >> 16) & mask_8[mask_index];
		uint64_t out2 = (head >> 24) & mask_8[mask_index];

		// Check to see if we are on a valid oct
		if ((head >> 16) & mask_8[mask_index]) {

			// Check to see if it is a leaf
			if ((head >> 24) & mask_8[mask_index]) {

				// If it is, then we cannot traverse further as CP's won't have been generated
				return true;
			}

			// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
			scale++;
			dimension /= 2;

			// Count the number of valid octs that come before and add it to the index to get the position
			int count = count_bits((uint8_t)(head >> 16) & count_mask_8[mask_index]);

			// Because we are getting the position at the first child we need to back up one
			// Or maybe it's because my count bits function is wrong...
			index = (head & child_pointer_mask) + count - 1;
			head = blob[index];

			// Increment the parent stack position and put the new oct node as the parent
			parent_stack_position++;
			parent_stack[parent_stack_position] = head;

		}
		else {
			// If the oct was not valid, then no CP's exists any further
			// This implicitly says that if it's non-valid then it must be a leaf!!

			// It appears that the traversal is now working but I need
			// to focus on how to now take care of the end condition.
			// Currently it adds the last parent on the second to lowest
			// oct CP. Not sure if thats correct
			return false;
		}
	}

	return true;
}

void Octree::print_block(int block_pos) {

	std::stringstream sss;
	for (int i = block_pos; i < (int)pow(2, 15); i++) {
		PrettyPrintUINT64(blob[i], &sss);
		sss << "\n";
	}
	DumpLog(&sss, "raw_data.txt");
	
}


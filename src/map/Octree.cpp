#include "map/Octree.h"

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

	// Struct that holds the state necessary to continue the traversal from the found voxel
	oct_state state;

	// push the root node to the parent stack
	uint64_t head = blob[root_index];
	state.parent_stack[state.parent_stack_position] = head;

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
			state.idx_stack[state.scale] |= idx_set_x_mask;

		}
		if (position.y >= (dimension / 2) + quad_position.y) {

			quad_position.y |= (dimension / 2);

			mask_index += 2;

			state.idx_stack[state.scale] ^= idx_set_y_mask;

		}
		if (position.z >= (dimension / 2) + quad_position.z) {

			quad_position.z += (dimension / 2);

			mask_index += 4;

			state.idx_stack[state.scale] |= idx_set_z_mask;

		}

		// Check to see if we are on a valid oct
		if ((head >> 16) & mask_8[mask_index]) {

			// Check to see if it is a leaf
			if ((head >> 24) & mask_8[mask_index]) {

				// If it is, then we cannot traverse further as CP's won't have been generated
				return true;
			}

			// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
			state.scale++;
			dimension /= 2;

			// Count the number of valid octs that come before and add it to the index to get the position
			// Negate it by one as it counts itself
			int count = count_bits((uint8_t)(head >> 16) & count_mask_8[mask_index]) - 1;

			// access the element at which head points to and then add the specified number of indices
			// to get to the correct child descriptor
			head = blob[(head & child_pointer_mask) + count];

			// Increment the parent stack position and put the new oct node as the parent
			state.parent_stack_position++;
			state.parent_stack[state.parent_stack_position] = head;

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


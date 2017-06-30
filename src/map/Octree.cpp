#include "map/Octree.h"

Octree::Octree() {

	// initialize the the buffers to 0's
	trunk_buffer		= new uint64_t[buffer_size]();
	descriptor_buffer	= new uint64_t[buffer_size]();
	attachment_lookup	= new uint32_t[buffer_size]();
	attachment_buffer	= new uint64_t[buffer_size]();

}


void Octree::Generate(char* data, sf::Vector3i dimensions) {

	// Launch the recursive generator at (0,0,0) as the first point
	// and the octree dimension as the initial block size
	std::tuple<uint64_t, uint64_t> root_node = GenerationRecursion(data, dimensions, sf::Vector3i(0, 0, 0), OCT_DIM/2);

	// ========= DEBUG ==============
	PrettyPrintUINT64(std::get<0>(root_node), &output_stream);
	output_stream << "    " << OCT_DIM << "    " << counter++ << std::endl;
	// ==============================
	
	// ============= TEMP!!! ===================
	if (stack_pos - 1 > stack_pos) {
		global_pos -= stack_pos;
		stack_pos = 0x8000;
	}
	else {
		stack_pos -= 1;
	}

	memcpy(&descriptor_buffer[descriptor_buffer_position], &std::get<0>(root_node), sizeof(uint64_t));
	descriptor_buffer_position--;

	// ========================================

	DumpLog(&output_stream, "raw_output.txt");

	output_stream.str("");

	for (int i = 0; i < buffer_size; i++) {
		PrettyPrintUINT64(descriptor_buffer[i], &output_stream);
	}

	DumpLog(&output_stream, "raw_data.txt");

}

bool Octree::get_voxel(sf::Vector3i position) {

	// Struct that holds the state necessary to continue the traversal from the found voxel
	oct_state state;

	// push the root node to the parent stack
	uint64_t head = descriptor_buffer[root_index];
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
			head = descriptor_buffer[(head & child_pointer_mask) + count];

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
		PrettyPrintUINT64(descriptor_buffer[i], &sss);
		sss << "\n";
	}
	DumpLog(&sss, "raw_data.txt");

}

std::tuple<uint64_t, uint64_t> Octree::GenerationRecursion(char* data, sf::Vector3i dimensions, sf::Vector3i pos, unsigned int voxel_scale) {


	// The 8 subvoxel coords starting from the 1th direction, the direction of the origin of the 3d grid
	// XY, Z++, XY
	std::vector<sf::Vector3i> v = {
		sf::Vector3i(pos.x              , pos.y              , pos.z),
		sf::Vector3i(pos.x + voxel_scale, pos.y              , pos.z),
		sf::Vector3i(pos.x              , pos.y + voxel_scale, pos.z),
		sf::Vector3i(pos.x + voxel_scale, pos.y + voxel_scale, pos.z),
		sf::Vector3i(pos.x              , pos.y              , pos.z + voxel_scale),
		sf::Vector3i(pos.x + voxel_scale, pos.y              , pos.z + voxel_scale),
		sf::Vector3i(pos.x              , pos.y + voxel_scale, pos.z + voxel_scale),
		sf::Vector3i(pos.x + voxel_scale, pos.y + voxel_scale, pos.z + voxel_scale)
	};

	// A tuple holding the child descriptor that we're going to fill out and the
	// absolute position of it within the descriptor buffer
	std::tuple<uint64_t, uint64_t> descriptor_and_position(0, 0);

	// If we hit the 1th voxel scale then we need to query the 3D grid
	// and get the voxel at that position. I assume in the future when I
	// want to do chunking / loading of raw data I can edit the voxel access
	if (voxel_scale == 1) {
		
		// Setting the individual valid mask bits
		// These don't bound check, should they?
		for (int i = 0; i < v.size(); i++) {
			if (get1DIndexedVoxel(data, dimensions, v.at(i)))
				SetBit(i + 16, &std::get<0>(descriptor_and_position));
		}

		// We are querying leafs, so we need to fill the leaf mask
		std::get<0>(descriptor_and_position) |= 0xFF000000;

		// The CP will be left blank, contour mask and ptr will need to 
		// be added here later
		return descriptor_and_position;

	}

	std::vector<std::tuple<uint64_t, uint64_t>> descriptor_position_array;

	// Generate down the recursion, returning the descriptor of the current node
	for (int i = 0; i < v.size(); i++) {

		std::tuple<uint64_t, uint64_t> child(0, 0);

		// Get the child descriptor from the i'th to 8th subvoxel
		child = GenerationRecursion(data, dimensions, v.at(i), voxel_scale / 2);

		// =========== Debug ===========
		PrettyPrintUINT64(std::get<0>(child), &output_stream);
		output_stream << "    " << voxel_scale << "    " << counter++ << std::endl;
		// =============================

		// If the child is a leaf (contiguous) of non-valid values
		if (IsLeaf(std::get<0>(child)) && !CheckLeafSign(std::get<0>(child))) {
			// Leave the valid mask 0, set leaf mask to 1
			SetBit(i + 16 + 8, &std::get<0>(descriptor_and_position));
		}

		// If the child is valid and not a leaf
		else {

			// Set the valid mask, and add it to the descriptor array
			SetBit(i + 16, &std::get<0>(descriptor_and_position));
			descriptor_position_array.push_back(child);
		}
	}
	
	// We are working bottom up so we need to subtract from the stack position
	// the amount of elements we want to use
	
	int worst_case_insertion_size = descriptor_position_array.size() * 2;

	// check to see if we exceeded this page header, if so set the header and move the global position
	if (page_header_counter - worst_case_insertion_size <= 0) {
		
		// Jump to the page headers position and reset the counter
		descriptor_buffer_position -= 0x8000 - page_header_counter;
		page_header_counter = 0x8000;

		// Fill the space with blank
		memcpy(&descriptor_buffer[descriptor_buffer_position], &current_info_section_position, sizeof(uint64_t));

		descriptor_buffer_position--;

	} 

	unsigned int far_pointer_count = 0;
	uint64_t far_pointer_block_position = descriptor_buffer_position;

	// Count the far pointers we need to allocate 
	for (int i = 0; i < descriptor_position_array.size(); i++) {
	
		// this is not the actual relative distance write, so we pessimistically guess that we will have
		// the worst relative distance via the insertion size

		int relative_distance = std::get<1>(descriptor_position_array.at(i)) - (descriptor_buffer_position - worst_case_insertion_size);

		// check to see if we tripped the far pointer
		if (relative_distance > 0x8000) {

			// This is writing the ABSOLUTE POSITION for far pointers, is this what I want?
			memcpy(&descriptor_buffer[descriptor_buffer_position], &std::get<1>(descriptor_position_array.at(i)), sizeof(uint64_t));
			descriptor_buffer_position--;
			page_header_counter--;

			far_pointer_count++;
		}
	}

	// We gotta go backwards as memcpy of a vector can be emulated by starting from the rear
	for (int i = 0; i < descriptor_position_array.size(); i++) {
		
		// just gonna redo the far pointer check loosing a couple of cycles but oh well
		int relative_distance = std::get<1>(descriptor_position_array.at(i)) - descriptor_buffer_position;

		uint64_t descriptor = std::get<0>(descriptor_position_array.at(i));

		// check to see if the 
		if (relative_distance > 0x8000) {

			descriptor |= far_bit_mask;
			descriptor |= far_pointer_block_position;

			far_pointer_block_position--;
		
		} else if (relative_distance > 0) {
		
			descriptor |= (uint64_t)relative_distance;
				
		}

		// We have finished building the CD so we push it onto the buffer
		memcpy(&descriptor_buffer[descriptor_buffer_position], &descriptor, sizeof(uint64_t));
		descriptor_buffer_position--;
		page_header_counter--;

	}

	std::get<1>(descriptor_and_position) = descriptor_buffer_position + 1;

	// Return the node up the stack
	return descriptor_and_position;
}

char Octree::get1DIndexedVoxel(char* data, sf::Vector3i dimensions, sf::Vector3i position) {	
	return data[position.x + OCT_DIM * (position.y + OCT_DIM * position.z)];
}



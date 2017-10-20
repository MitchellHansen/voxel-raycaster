#include "map/Octree.h"

Octree::Octree() {

	// initialize the the buffers to 0's
	descriptor_buffer	= new uint64_t[buffer_size]();
	attachment_lookup	= new uint32_t[buffer_size]();
	attachment_buffer	= new uint64_t[buffer_size]();
}


void Octree::Generate(char* data, sf::Vector3i dimensions) {

	oct_dimensions = dimensions.x;

	// Launch the recursive generator at (0,0,0) as the first point
	// and the octree dimension as the initial block size
	std::tuple<uint64_t, uint64_t> root_node = GenerationRecursion(data, dimensions, sf::Vector3i(0, 0, 0), oct_dimensions/2);

	// ========= DEBUG ==============
	PrettyPrintUINT64(std::get<0>(root_node), &output_stream);
	output_stream << "    " << oct_dimensions << "    " << counter++ << std::endl;
	// ==============================

    // set the root nodes relative pointer to 1 because the next element will be the top of the tree, and push to the stack
    std::get<0>(root_node) |= 1;    
	memcpy(&descriptor_buffer[descriptor_buffer_position], &std::get<0>(root_node), sizeof(uint64_t));
	
    root_index = descriptor_buffer_position;
    descriptor_buffer_position--;

	DumpLog(&output_stream, "raw_output.txt");
	output_stream.str("");

	for (int i = 0; i < buffer_size; i++) {
		PrettyPrintUINT64(descriptor_buffer[i], &output_stream);
	}

	DumpLog(&output_stream, "raw_data.txt");

}

OctState Octree::GetVoxel(sf::Vector3i position) {

	// Struct that holds the state necessary to continue the traversal from the found voxel
	OctState state;

	state.oct_pos = sf::Vector3i(0,0,0);

	// push the root node to the parent stack
    uint64_t current_index = root_index;
    uint64_t head = descriptor_buffer[current_index];
    
    //PrettyPrintUINT64(head);
	state.parent_stack[state.parent_stack_position] = head;
	state.parent_stack_index[state.parent_stack_position] = current_index;
	// Set our initial dimension and the position at the corner of the oct to keep track of our position
	int dimension = oct_dimensions;

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

		// Do the logic steps to find which sub oct we step down into
		if (position.x >= (dimension / 2) + state.oct_pos.x) {

			// Set our voxel position to the (0,0) of the correct oct
			state.oct_pos.x += (dimension / 2);

			// Set the idx to represent the move
			state.idx_stack[state.scale] |= idx_set_x_mask;

		}
		if (position.y >= (dimension / 2) + state.oct_pos.y) {

			// TODO What the hell is going on with the or operator on this one!??!?!?!
			state.oct_pos.y += (dimension / 2);

            // TODO What is up with the XOR operator that was on this one?
			state.idx_stack[state.scale] |= idx_set_y_mask;

		}
		if (position.z >= (dimension / 2) + state.oct_pos.z) {

			state.oct_pos.z += (dimension / 2);

			state.idx_stack[state.scale] |= idx_set_z_mask;

		}

		// Our count mask matches the way we index our idx so we can just 
		// copy it over
		int mask_index = state.idx_stack[state.scale];

		// Check to see if we are on a valid oct
		if ((head >> 16) & mask_8[mask_index]) {

			// Check to see if it is a leaf
			if ((head >> 24) & mask_8[mask_index]) {

				// If it is, then we cannot traverse further as CP's won't have been generated
				state.found = 1;
				return state;
			}

			// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
			state.scale++;
			dimension /= 2;

			// Count the number of valid octs that come before and add it to the index to get the position
			// Negate it by one as it counts itself
			int count = count_bits((uint8_t)(head >> 16) & count_mask_8[mask_index]) - 1;

			// access the far point at which the head points too. Determine it's value, and add
			// a count of the valid bits to the index
			if (far_bit_mask & descriptor_buffer[current_index]) {
				int far_pointer_index = current_index + (head & child_pointer_mask);
				current_index = descriptor_buffer[far_pointer_index] + count;
			} 
			// access the element at which head points to and then add the specified number of indices
			// to get to the correct child descriptor
			else {
				current_index = current_index + (head & child_pointer_mask) + count;
			}

			head = descriptor_buffer[current_index];

			// Increment the parent stack position and put the new oct node as the parent
			state.parent_stack_position++;
			state.parent_stack[state.parent_stack_position] = head;
			state.parent_stack_index[state.parent_stack_position] = current_index;
		}
		else {
			// If the oct was not valid, then no CP's exists any further
			// This implicitly says that if it's non-valid then it must be a leaf!!

			// It appears that the traversal is now working but I need
			// to focus on how to now take care of the end condition.
			// Currently it adds the last parent on the second to lowest
			// oct CP. Not sure if thats correct
			state.found = 0;
			return state;
		}
	}

	state.found = 1;
	return state;
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

	// Array of <descriptors, position>
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
	// the amount of elements we want to use. In the worst case this will be 
	// a far pointer for ever descriptor (size * 2)
	
	int worst_case_insertion_size = descriptor_position_array.size() * 2;

	// check to see if we exceeded this page header, if so set the header and move the global position
	if (page_header_counter - worst_case_insertion_size <= 0) {
		
		// Jump to the page headers position and reset the counter
		descriptor_buffer_position -= page_header_counter;
		page_header_counter = 0x8000;

		// Fill the space with blank
		memcpy(&descriptor_buffer[descriptor_buffer_position], &current_info_section_position, sizeof(uint64_t));

		descriptor_buffer_position--;

	} 

	unsigned int far_pointer_count = 0;

	// If looking "up" to zero, the far ptr is entered first before the cp block. Get it's position
	uint64_t far_pointer_block_position = descriptor_buffer_position;

	// Count the far pointers we need to allocate 
	for (int i = descriptor_position_array.size() - 1; i >= 0; i--) {
	
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
	for (int i = descriptor_position_array.size() - 1; i >= 0; i--) {
		
		// just gonna redo the far pointer check loosing a couple of cycles but oh well
		int relative_distance = std::get<1>(descriptor_position_array.at(i)) - descriptor_buffer_position;

		uint64_t descriptor = std::get<0>(descriptor_position_array.at(i));

		// check to see if the 
		if (relative_distance > 0x8000) {

			descriptor |= far_bit_mask;
			// The distance from this cp to the far ptr
			descriptor |= far_pointer_block_position - descriptor_buffer_position;

			far_pointer_block_position--;
		
		} else if (relative_distance > 0) {
		
			descriptor |= (uint64_t)relative_distance;
				
		}

		// We have finished building the CD so we push it onto the buffer
		memcpy(&descriptor_buffer[descriptor_buffer_position], &descriptor, sizeof(uint64_t));
		descriptor_buffer_position--;
		page_header_counter--;
	}

    // The position this descriptor points to is the last one written to the stack. AKA
    // the current stack position (empty slot) plus one
	std::get<1>(descriptor_and_position) = descriptor_buffer_position + 1;

	// Return the node up the stack
	return descriptor_and_position;
}

char Octree::get1DIndexedVoxel(char* data, sf::Vector3i dimensions, sf::Vector3i position) {	
	return data[position.x + oct_dimensions * (position.y + oct_dimensions * position.z)];
}

bool Octree::Validate(char* data, sf::Vector3i dimensions){

	for (int x = 0; x < dimensions.x; x++) {
		for (int y = 0; y < dimensions.y; y++) {
			for (int z = 0; z < dimensions.z; z++) {

				sf::Vector3i pos(x, y, z);

                char arr_val = get1DIndexedVoxel(data, dimensions, pos);
                char oct_val = GetVoxel(pos).found;

				if (arr_val != oct_val) {
					std::cout << "X: " << pos.x << " Y: " << pos.y << " Z: " << pos.z << "   ";
                    std::cout << (int)arr_val << "  :  " << (int)oct_val << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

unsigned int Octree::getDimensions() {
	return oct_dimensions;
}


std::vector<std::tuple<sf::Vector3i, char>> Octree::CastRayOctree(
	sf::Vector2f cam_dir,
	sf::Vector3f cam_pos
) {

	// Setup the voxel coords from the camera origin
	sf::Vector3i voxel(cam_pos);

	// THIS DOES NOT HAVE TO RETURN TRUE ON FOUND
	// This function when passed an "air" voxel will return as far down
	// the IDX stack as it could go. We use this oct-level to determine
	// our first position and jump. Updating it as we go
	OctState traversal_state = GetVoxel(voxel);

	std::vector<std::tuple<sf::Vector3i, char>> travel_path;

	sf::Vector3f ray_dir(1, 0, 0);

	// Pitch
	ray_dir = sf::Vector3f(
		ray_dir.z * sin(cam_dir.x) + ray_dir.x * cos(cam_dir.x),
		ray_dir.y,
		ray_dir.z * cos(cam_dir.x) - ray_dir.x * sin(cam_dir.x)
	);

	// Yaw
	ray_dir = sf::Vector3f(
		ray_dir.x * cos(cam_dir.y) - ray_dir.y * sin(cam_dir.y),
		ray_dir.x * sin(cam_dir.y) + ray_dir.y * cos(cam_dir.y),
		ray_dir.z
	);

	// correct for the base ray pointing to (1, 0, 0) as (0, 0). Should equal (1.57, 0)
	ray_dir = sf::Vector3f(
		static_cast<float>(ray_dir.z * sin(-1.57) + ray_dir.x * cos(-1.57)),
		static_cast<float>(ray_dir.y),
		static_cast<float>(ray_dir.z * cos(-1.57) - ray_dir.x * sin(-1.57))
	);


	// Setup the voxel step based on what direction the ray is pointing
	sf::Vector3i voxel_step(1, 1, 1);

	voxel_step.x *= (ray_dir.x > 0) - (ray_dir.x < 0);
	voxel_step.y *= (ray_dir.y > 0) - (ray_dir.y < 0);
	voxel_step.z *= (ray_dir.z > 0) - (ray_dir.z < 0);

	// set the jump multiplier based on the traversal state vs the log base 2 of the maps dimensions
	int jump_power = log2(oct_dimensions) - traversal_state.scale;


	// Delta T is the units a ray must travel along an axis in order to
	// traverse an integer split
	sf::Vector3f delta_t(
		fabs(1.0f / ray_dir.x),
		fabs(1.0f / ray_dir.y),
		fabs(1.0f / ray_dir.z)
	);

	delta_t *= static_cast<float>(jump_power);

	// Intersection T is the collection of the next intersection points
	// for all 3 axis XYZ. We take the full positive cardinality when
	// subtracting the floor, so we must transfer the sign over from
	// the voxel step
	
	sf::Vector3f intersection_t(
		delta_t.x * (cam_pos.y - ceil(cam_pos.x)) * voxel_step.x,
		delta_t.y * (cam_pos.x - ceil(cam_pos.y)) * voxel_step.y,
		delta_t.z * (cam_pos.z - ceil(cam_pos.z)) * voxel_step.z
	);

	// When we transfer the sign over, we get the correct direction of 
	// the offset, but we merely transposed over the value instead of mirroring
	// it over the axis like we want. So here, isless returns a boolean if intersection_t
	// is less than 0 which dictates whether or not we subtract the delta which in effect
	// mirrors the offset
	intersection_t.x -= delta_t.x * (std::isless(intersection_t.x, 0.0f));
	intersection_t.y -= delta_t.y * (std::isless(intersection_t.y, 0.0f));
	intersection_t.z -= delta_t.z * (std::isless(intersection_t.z, 0.0f));

	int dist = 0;
	sf::Vector3i face_mask(0, 0, 0);
	int voxel_data = 0;

	// Andrew Woo's raycasting algo
	do {

		// check which direction we step in
		face_mask.x = intersection_t.x <= std::min(intersection_t.y, intersection_t.z);
		face_mask.y = intersection_t.y <= std::min(intersection_t.z, intersection_t.x);
		face_mask.z = intersection_t.z <= std::min(intersection_t.x, intersection_t.y);

		// Increment the selected directions intersection, abs the face_mask to stay within the algo constraints
		intersection_t.x += delta_t.x * fabs(face_mask.x);
		intersection_t.y += delta_t.y * fabs(face_mask.y);
		intersection_t.z += delta_t.z * fabs(face_mask.z);

		// step the voxel direction
		voxel.x += voxel_step.x * face_mask.x * jump_power;
		voxel.y += voxel_step.y * face_mask.y * jump_power;
		voxel.z += voxel_step.z * face_mask.z * jump_power;

		uint8_t prev_val = traversal_state.idx_stack[traversal_state.scale];
		uint8_t this_face_mask = 0;

		// Check the voxel face that we traversed
		// and increment the idx in the idx stack
		if (face_mask.x) {
			this_face_mask = Octree::idx_set_x_mask;
		}
		else if (face_mask.y) {
			this_face_mask = Octree::idx_set_y_mask;
		}
		else if (face_mask.z) {
			this_face_mask = Octree::idx_set_z_mask;
		}

		traversal_state.idx_stack[traversal_state.scale] ^= this_face_mask;

		// Mask index is the 1D index'd value of the idx for interaction with the valid / leaf masks
		int mask_index = traversal_state.idx_stack[traversal_state.scale];

		// Whether or not the next oct we want to enter in the current CD's valid mask is 1 or 0
		bool is_valid = false;
		
		// TODO: Rework this logic so we don't have this bodgy if
		if (mask_index > prev_val)
			is_valid = (traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & mask_8[mask_index];
		
		// Check to see if the idx increased or decreased	
		// If it decreased
		//		Pop up the stack until the oct that the idx flip is valid and we landed on a valid oct
		while (mask_index < prev_val || !is_valid) {

			jump_power *= 2;

			// Keep track of the 0th edge of out current oct
			traversal_state.oct_pos.x = floor(voxel.x / 2) * jump_power;
			traversal_state.oct_pos.y = floor(voxel.y / 2) * jump_power;
			traversal_state.oct_pos.z = floor(voxel.z / 2) * jump_power;

			// Clear and pop the idx stack
			traversal_state.idx_stack[traversal_state.scale] = 0;

			// Scale is now set to the oct above. Be wary of this
			traversal_state.scale--;

			// Update the prev_val for our new idx
			prev_val = traversal_state.idx_stack[traversal_state.scale];

			// Clear and pop the parent stack, maybe off by one error?
			traversal_state.parent_stack_index[traversal_state.parent_stack_position] = 0;
			traversal_state.parent_stack[traversal_state.parent_stack_position] = 0;
			traversal_state.parent_stack_position--;

			// Set the current CD to the one on top of the stack
			traversal_state.current_descriptor =
				traversal_state.parent_stack[traversal_state.parent_stack_position];

			// Apply the face mask to the new idx for the while check
			traversal_state.idx_stack[traversal_state.scale] ^= this_face_mask;

			// Get the mask index of the new idx and check the valid status
			mask_index = traversal_state.idx_stack[traversal_state.scale];
			is_valid = (traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & mask_8[mask_index];
		}

		// At this point parent_stack[position] is at the CD of an oct with a valid oct at the leaf indicated by the current
		// idx in the idx stack scale

		// While we haven't bottomed out and the oct we're looking at is valid
		while (jump_power > 1 && is_valid) {

			// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
			traversal_state.scale++;
			jump_power /= 2;

			// Count the number of valid octs that come before and add it to the index to get the position
			// Negate it by one as it counts itself
			int count = count_bits((uint8_t)(traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & count_mask_8[mask_index]) - 1;

			// If this CD had the far bit set
			if (far_bit_mask & descriptor_buffer[traversal_state.parent_stack_index[traversal_state.parent_stack_position]]) {

				// access the far point at which the head points too. Determine it's value, and add
				// the count of the valid bits in the current CD to the index
				uint64_t far_pointer_index = 
					traversal_state.parent_stack_index[traversal_state.parent_stack_position] + // current index +
					(traversal_state.parent_stack[traversal_state.parent_stack_position] & child_pointer_mask); // the relative prt to the far ptr

				// Get the absolute ptr from the far ptr and add the count to get the CD that we want
				traversal_state.parent_stack_index[traversal_state.parent_stack_position + 1] = descriptor_buffer[far_pointer_index] + count;
			}
			// If this CD doesn't have the far bit set, access the element at which head points to 
			// and then add the specified number of indices to get to the correct child descriptor
			else {
				traversal_state.parent_stack_index[traversal_state.parent_stack_position + 1] = 
					traversal_state.parent_stack_index[traversal_state.parent_stack_position] + // The current index to this CD
					(traversal_state.parent_stack[traversal_state.parent_stack_position] & child_pointer_mask) + count; // The relative dist + the number of bits that were valid
			}

			// Now that we have the index set we can increase our parent stack position to the next level and
			// retrieve the value of its CD
			traversal_state.parent_stack_position++;
			traversal_state.parent_stack[traversal_state.parent_stack_position] = descriptor_buffer[traversal_state.parent_stack_index[traversal_state.parent_stack_position]];

			// Unlike the single shot DFS, it makes a bit more sense to have this at the tail of the while loop
			// Do the logic steps to find which sub oct we step down into
			if (voxel.x >= (jump_power / 2) + traversal_state.oct_pos.x) {

				// Set our voxel position to the (0,0) of the correct oct
				traversal_state.oct_pos.x += (jump_power / 2);

				// Set the idx to represent the move
				traversal_state.idx_stack[traversal_state.scale] |= idx_set_x_mask;

			}
			if (voxel.y >= (jump_power / 2) + traversal_state.oct_pos.y) {

				traversal_state.oct_pos.y += (jump_power / 2);
				traversal_state.idx_stack[traversal_state.scale] |= idx_set_y_mask;
			}
			if (voxel.z >= (jump_power / 2) + traversal_state.oct_pos.z) {

				traversal_state.oct_pos.z += (jump_power / 2);
				traversal_state.idx_stack[traversal_state.scale] |= idx_set_z_mask;
			}

			// Update the mask index with the new voxel we walked down to, and then check it's valid status
			mask_index = traversal_state.idx_stack[traversal_state.scale];
			is_valid = (traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & mask_8[mask_index];

		}

		// At this point we are the furthest down the oct we can get to the voxel.xyz position. This can either
		// be at the min oct level with 1:1 traversal, or as high as the max oct. Our jump power has been updated accordingly,
		// but we need to update our intersection T's





		//if (voxel.x >= map_dim->x || voxel.y >= map_dim->y || voxel.z >= map_dim->z) {
		//	return travel_path;
		//}
		//if (voxel.x < 0 || voxel.y < 0 || voxel.z < 0) {
		//	return travel_path;
		//}

		// If we hit a voxel
		//voxel_data = map[voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z))];
		//		voxel_data = getVoxel(voxel);
		travel_path.push_back(std::make_tuple(voxel, voxel_data));

		if (voxel_data != 0)
			return travel_path;


	} while (++dist < 700.0f);

	return travel_path;
}

const uint8_t Octree::mask_8[8] = {
	0x1,  0x2,  0x4,  0x8,
	0x10, 0x20, 0x40, 0x80
};

const uint8_t Octree::count_mask_8[8] = {
	0x1,  0x3,  0x7,  0xF,
	0x1F, 0x3F, 0x7F, 0xFF
};

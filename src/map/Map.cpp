#include "map/Map.h"


Map::Map(uint32_t dimensions) {

	srand(time(nullptr));

	voxel_data = new char[dimensions * dimensions * dimensions];

	for (uint64_t i = 0; i < dimensions * dimensions * dimensions; i++) {
		if (rand() % 25 < 2)
			voxel_data[i] = 1;
		else
			voxel_data[i] = 0;
	}

	generate_octree(dimensions);
}


void Map::dump_logs() {

	octree.print_block(0);
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

		uint64_t child_descriptor = 0;

		// Setting the individual valid mask bits
		// These don't bound check, should they?
		for (int i = 0; i < v.size(); i++) {
			if (getVoxel(v.at(i)))
				SetBit(i + 16, &child_descriptor);
		}

		// We are querying leafs, so we need to fill the leaf mask
		child_descriptor |= 0xFF000000;

		// The CP will be left blank, contour mask and ptr will need to 
		// be added here later
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
	child_descriptor |= octree.copy_to_stack(descriptor_array, voxel_scale);

	// Free space may also be allocated here as well
	
	// Return the node up the stack
	return child_descriptor;
}

void Map::generate_octree(unsigned int dimensions) {

	// Launch the recursive generator at (0,0,0) as the first point
	// and the octree dimension as the initial block size
	uint64_t root_node = generate_children(sf::Vector3i(0, 0, 0), OCT_DIM/2);

	// ========= DEBUG ==============
	PrettyPrintUINT64(root_node, &output_stream);
	output_stream << "    " << OCT_DIM << "    " << counter++ << std::endl;
	// ==============================

	octree.root_index = octree.copy_to_stack(std::vector<uint64_t>{root_node}, OCT_DIM);

	// Dump the debug log
	DumpLog(&output_stream, "raw_output.txt");

}

void Map::setVoxel(sf::Vector3i world_position, int val) {

}

bool Map::getVoxelFromOctree(sf::Vector3i position)
{
	return octree.get_voxel(position);
}

bool Map::getVoxel(sf::Vector3i pos){

	if (voxel_data[pos.x + OCT_DIM * (pos.y + OCT_DIM * pos.z)]) {
		return true;
	} else {
		return false;
	}
}

bool Map::test() {

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

				bool arr1 = getVoxel(pos);
			}
		}
	}

	std::cout << "Array linear xyz access : ";
	std::cout << timer.restart().asMicroseconds() << " microseconds" << std::endl;

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


	return true;
}


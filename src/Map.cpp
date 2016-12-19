#include "Map.h"

int BitCount(unsigned int u) {
	unsigned int uCount;

	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

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

	load_unload(position);

	for (int i = 0; i < OCT_DIM * OCT_DIM * OCT_DIM; i++) {
		if (rand() % 8 > 2)
			voxel_data[i] = 0;
		else
			voxel_data[i] = 1;
	}
}

uint64_t Map::generate_children(sf::Vector3i pos, int dim) {


	// The 8 subvoxel coords starting from the 1th direction, the direction of the origin of the 3d grid
	// XY, Z++, XY
	std::vector<sf::Vector3i> v = { 
		sf::Vector3i(pos.x, pos.y, pos.z),
		sf::Vector3i(pos.x + dim, pos.y, pos.z),
		sf::Vector3i(pos.x, pos.y + dim, pos.z),
		sf::Vector3i(pos.x + dim, pos.y + dim, pos.z),
		sf::Vector3i(pos.x, pos.y, pos.z + dim),
		sf::Vector3i(pos.x + dim, pos.y, pos.z + dim),
		sf::Vector3i(pos.x, pos.y + dim, pos.z + dim),
		sf::Vector3i(pos.x + dim, pos.y + dim, pos.z + dim) 
	};

	if (dim == 1) {

		// Return the base 2x2 leaf node
		uint64_t tmp = 0;

		// These don't bound check, should they?
		// Setting the individual valid mask bits
		for (int i = 0; i < v.size(); i++) {
			if (getVoxel(v.at(i)))
				SetBit(i + 16, &tmp);
		}

		// Set the leaf mask to full
		tmp |= 0xFF000000;

		// The CP will be left blank, contours will be added maybe
		return tmp;

	}
	else {

		// 2339 is the iterative anomoly
		// 30454 is the stack anomoly

		uint64_t tmp = 0;
		uint64_t child = 0;

		std::vector<uint64_t> children;

		// Generate down the recursion, returning the descriptor of the current node
		for (int i = 0; i < v.size(); i++) {

			// Get the child descriptor from the i'th to 8th subvoxel
			child = generate_children(v.at(i), dim / 2);

			PrettyPrintUINT64(child, &ss);
			ss << "    " << dim << "    " << counter++ << std::endl;

			if (IsLeaf(child)) {
				if (CheckLeafSign(child))
					SetBit(i + 16, &tmp);

				SetBit(i + 16 + 8, &tmp);
			}

			else {
				SetBit(i + 16, &tmp);
				children.push_back(child);
			}
		}

		// Now put those values onto the block stack, it returns the 
		// 16 bit topmost pointer to the block. The 16th bit being
		// a switch to jump to a far pointer.
		int y = 0;
		tmp |= a.copy_to_stack(children);

		if ((tmp & 0xFFFFFFFF00000000) != 0) {
			abort();
		}
		
		return tmp;

	}

	return 0;
}

void Map::generate_octree() {

	generate_children(sf::Vector3i(0, 0, 0), OCT_DIM/2);
	DumpLog(&ss, "raw_output.txt");

	std::stringstream sss;
	for (int i = 0; i < (int)pow(2, 15); i++) {
		PrettyPrintUINT64(a.dat[i], &sss);
		sss << "\n";
	}
	DumpLog(&sss, "raw_data.txt");

	// levels defines how many levels to traverse before we hit raw data
	// Will be the map width I presume. Will still need to handle how to swap in and out data.
	// Possible have some upper static nodes that will stay full regardless of contents?
	int levels = static_cast<int>(log2(64));


	std::list<int> parent_stack;

	int byte_pos = 0;

	unsigned int parent = 0;
	for (int i = 0; i < 16; i++) {
		parent ^= 1 << i;
	}

	unsigned int leafmask = 255;
	unsigned int validmask = leafmask << 8;

	parent &= validmask;
	parent &= leafmask;

	std::cout << BitCount(parent & leafmask);

	unsigned int children[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

}

void Map::load_unload(sf::Vector3i world_position) {
	
	sf::Vector3i chunk_pos(world_to_chunk(world_position));
	
	//Don't forget the middle chunk
	if (chunk_map.find(chunk_pos) == chunk_map.end()) {
		chunk_map[chunk_pos] = Chunk(5);
	}

	for (int x = chunk_pos.x - chunk_radius / 2; x < chunk_pos.x + chunk_radius / 2; x++) {
		for (int y = chunk_pos.y - chunk_radius / 2; y < chunk_pos.y + chunk_radius / 2; y++) {
			for (int z = chunk_pos.z - chunk_radius / 2; z < chunk_pos.z + chunk_radius / 2; z++) {

				if (chunk_map.find(sf::Vector3i(x, y, z)) == chunk_map.end()) {
					chunk_map.emplace(sf::Vector3i(x, y, z), Chunk(rand() % 6));
					//chunk_map[sf::Vector3i(x, y, z)] = Chunk(rand() % 6);
				}
			}
		}
	}
}

void Map::load_single(sf::Vector3i world_position) {
	sf::Vector3i chunk_pos(world_to_chunk(world_position));

	//Don't forget the middle chunk
	if (chunk_map.find(chunk_pos) == chunk_map.end()) {
		chunk_map[chunk_pos] = Chunk(0);
	}
}

sf::Vector3i Map::getDimensions() {
	return sf::Vector3i(0, 0, 0);
}

void Map::setVoxel(sf::Vector3i world_position, int val) {

	load_single(world_position);
	sf::Vector3i chunk_pos(world_to_chunk(world_position));
	sf::Vector3i in_chunk_pos(
		world_position.x % CHUNK_DIM,
		world_position.y % CHUNK_DIM,
		world_position.z % CHUNK_DIM
	);

	chunk_map.at(chunk_pos).voxel_data[in_chunk_pos.x + CHUNK_DIM * (in_chunk_pos.y + CHUNK_DIM * in_chunk_pos.z)] 
		= val;

}

char Map::getVoxel(sf::Vector3i pos){

	return voxel_data[pos.x + OCT_DIM * (pos.y + OCT_DIM * pos.z)];
}

void Chunk::set(int type) {
	for (int i = 0; i < CHUNK_DIM * CHUNK_DIM * CHUNK_DIM; i++) {
		voxel_data[i] = 0;
	}

	for (int x = 0; x < CHUNK_DIM; x+=2) {
		for (int y = 0; y < CHUNK_DIM; y+=2) {
			//list[x + dim.x * (y + dim.z * z)]
			voxel_data[x + CHUNK_DIM * (y + CHUNK_DIM * 1)] = type;
		}
	}
}

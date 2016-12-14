#include "Map.h"


Map::Map(sf::Vector3i position) {
	
	load_unload(position);

	for (int i = 0; i < 1024; i++) {
		block[i] = 0;
	}
}

int BitCount(unsigned int u) {
	unsigned int uCount;

	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

void SetBit(int position, char* c) {
	*c |= 1 << position;
}

void FlipBit(int position, char* c) {
	*c ^= 1 << position;
}

int GetBit(int position, char* c) {
	return (*c >> position) & 1;
}

void SetBit(int position, uint64_t* c) {
	*c |= 1 << position;
}

void FlipBit(int position, uint64_t* c) {
	*c ^= 1 << position;
}

int GetBit(int position, uint64_t* c) {
	return (*c >> position) & 1;
}

struct nonleaf {
   std::vector<nonleaf> children;
    char leaf_mask;
    char valid_mask;
};


uint64_t Map::generate_children(sf::Vector3i pos, int dim) {

	sf::Vector3i t1 = sf::Vector3i(pos.x, pos.y, pos.z);
	sf::Vector3i t2 = sf::Vector3i(pos.x + dim, pos.y, pos.z);
	sf::Vector3i t3 = sf::Vector3i(pos.x, pos.y + dim, pos.z);
	sf::Vector3i t4 = sf::Vector3i(pos.x + dim, pos.y + dim, pos.z);

	sf::Vector3i t5 = sf::Vector3i(pos.x, pos.y, pos.z + dim);
	sf::Vector3i t6 = sf::Vector3i(pos.x + dim, pos.y, pos.z + dim);
	sf::Vector3i t7 = sf::Vector3i(pos.x, pos.y + dim, pos.z + dim);
	sf::Vector3i t8 = sf::Vector3i(pos.x + dim, pos.y + dim, pos.z + dim);

	std::vector<uint64_t> cps;
	uint64_t tmp = 0;

	int cycle_num = cycle_counter;
	cycle_counter++;

	if (dim == 1) {

		// These don't bound check, should they?
		if (getVoxel(t1))
			SetBit(16, &tmp);
		if (getVoxel(t2))
			SetBit(17, &tmp);
		if (getVoxel(t3))
			SetBit(18, &tmp);
		if (getVoxel(t4))
			SetBit(19, &tmp);
		if (getVoxel(t5))
			SetBit(20, &tmp);
		if (getVoxel(t6))
			SetBit(21, &tmp);
		if (getVoxel(t7))
			SetBit(22, &tmp);
		if (getVoxel(t8))
			SetBit(23, &tmp);

		cps.push_back(tmp);

	}
	else {

		// Generate all 8 sub trees accounting for each of their unique positions

		tmp = generate_children(t1, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t2, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t3, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t4, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t5, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t6, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t7, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

		tmp = generate_children(t8, dim / 2);
		if (tmp != 0)
			cps.push_back(tmp);

	}

	a.reserve(cycle_num, cps);

	return 0;
}

void Map::generate_octree() {

	char* arr[8192];
	for (int i = 0; i < 8192; i++) {
		arr[i] = 0;
	}

	generate_children(sf::Vector3i(0, 0, 0), 64);

	int* dataset = new int[32 * 32 * 32];
	for (int i = 0; i < 32 * 32 * 32; i++) {
		dataset[0] = rand() % 2;
	}

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

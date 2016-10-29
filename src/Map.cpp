#include "Map.h"

Map::Map(sf::Vector3i position) {
	
	load_unload(position);
}

int BitCount(unsigned int u) {
	unsigned int uCount;

	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

struct leaf {
    leaf *children;
    char leaf_mask;
    char valid_mask;
    int level;
};

struct block {
	int header = 0;
	double* data = new double[1000];
};


void Map::generate_octree() {

	char* arr[8192];
	for (int i = 0; i < 8192; i++) {
		arr[i] = 0;
	}

	int* dataset = new int[32 * 32 * 32];
	for (int i = 0; i < 32 * 32 * 32; i++) {
		dataset[0] = i;
	}

	int level = static_cast<int>(log2(32));

	leaf top_node;
	top_node.level = level;

	for (int i = 0; i < 16 * 16 * 16; i++) {
		for (int i = 0; i < 8 * 8 * 8; i++) {
			for (int i = 0; i < 4 * 4 * 4; i++) {
				for (int i = 0; i < 2 * 2 * 2; i++) {

				}
			}
		}
	}



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

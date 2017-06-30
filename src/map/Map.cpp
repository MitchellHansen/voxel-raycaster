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

void Map::generate_octree(unsigned int dimensions) {
	
	octree.Generate(voxel_data, sf::Vector3i(dimensions, dimensions, dimensions));

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


#pragma once
#include <SFML/System/Vector3.hpp>
#include <functional>
#include <bitset>
#include <queue>
#include "util.hpp"
#include "map/Octree.h"

#define _USE_MATH_DEFINES
#include <math.h>

struct XYZHasher {
	std::size_t operator()(const sf::Vector3i& k) const {
		return ((std::hash<int>()(k.x)
			^ (std::hash<int>()(k.y) << 1)) >> 1)
			^ (std::hash<int>()(k.z) << 1);
	}
};

class Map {
public: 

	Map(uint32_t dimensions);

	void generate_octree();

	void setVoxel(sf::Vector3i position, int val);

	bool getVoxelFromOctree(sf::Vector3i position);

	bool getVoxel(sf::Vector3i pos);
	Octree a;

	void test_map();

private:

	// ======= DEBUG ===========
	int counter = 0;
	std::stringstream output_stream;
	// =========================

	uint64_t generate_children(sf::Vector3i pos, int dim);

	char* voxel_data;

};



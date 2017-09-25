#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Clock.hpp>
#include <functional>
#include <bitset>
#include <queue>
#include "util.hpp"
#include "map/Octree.h"
#include <time.h>
#include "map/Old_Map.h"

#define _USE_MATH_DEFINES
#include <math.h>

class Map {
public: 

	// Currently takes a 
	Map(uint32_t dimensions, Old_Map* array_map);

	// Sets a voxel in the 3D char dataset
	void setVoxel(sf::Vector3i position, int val);
	
	// Gets a voxel at the 3D position in the octree
	char getVoxel(sf::Vector3i pos);

	std::vector<std::tuple<sf::Vector3i, char>> CastRayOctree(
		Octree *octree,
		sf::Vector3i* map_dim,
		sf::Vector2f* cam_dir,
		sf::Vector3f* cam_pos
	);

	std::vector<std::tuple<sf::Vector3i, char>> CastRayCharArray(
		char *map,
		sf::Vector3i* map_dim,
		sf::Vector2f* cam_dir,
		sf::Vector3f* cam_pos
	);
	
	// Octree handles all basic octree operations
    Octree octree;

private:

	bool test_oct_arr_traversal(sf::Vector3i dimensions);

	// ======= DEBUG ===========
	int counter = 0;
	std::stringstream output_stream;
	
	// The 3D char dataset that is generated at runtime. This will be replaced by two different interactions.
	// The first a file loading function that loads binary octree data.
	// The second being an import tool which will allow Any -> Octree transformation.
	char* voxel_data;
	// =========================

};

// Might possibly use this struct for hashing XYZ chunk values into a dict for storage and loading
struct XYZHasher {
	std::size_t operator()(const sf::Vector3i& k) const {
		return ((std::hash<int>()(k.x)
			^ (std::hash<int>()(k.y) << 1)) >> 1)
			^ (std::hash<int>()(k.z) << 1);
	}
};

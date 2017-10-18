#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Clock.hpp>
#include <functional>
#include <bitset>
#include <queue>
#include "util.hpp"
#include "map/Octree.h"
#include <time.h>
#include "map/ArrayMap.h"

#define _USE_MATH_DEFINES
#include <math.h>



// MonolithicMap
//		Octree
//		Map

// Player
//		Camera
//		Movement interface
//		Subscription to joystick events?

// Player needs to have some way to query the map
//	Map could return collision result
//		Could handle multiple collision types, aabb, ray
//	player could query map and generate collision
//		Wouldn't need to make map more complex



class Map {
public: 

	// Currently takes a 
	Map(uint32_t dimensions);

	// Sets a voxel in the 3D char dataset
	void setVoxel(sf::Vector3i position, int val);
	
	// Gets a voxel at the 3D position in the octree
	char getVoxel(sf::Vector3i pos);

	// Return the position at which a generalized ray hits a voxel
	sf::Vector3f LongRayIntersection(sf::Vector3f origin, sf::Vector3f magnitude);
	
	// Return the voxels that a box intersects / contains
	std::vector<sf::Vector3i> BoxIntersection(sf::Vector3f origin, sf::Vector3f magnitude);
	
	// Return a normalized ray opposite of the intersected normals
	sf::Vector3f ShortRayIntersection(sf::Vector3f origin, sf::Vector3f magnitude);
	
	sf::Image GenerateHeightBitmap(sf::Vector3i dimensions);

	void ApplyHeightmap(sf::Image bitmap);

	// Octree handles all basic octree operations
    Octree octree;
	ArrayMap array_map;

private:

	// ======= DEBUG ===========
	int counter = 0;
	std::stringstream output_stream;
	
	sf::Vector3i dimensions;
	// =========================

	double Sample(int x, int y, double *height_map);
	void SetSample(int x, int y, double value, double *height_map);
	void SampleSquare(int x, int y, int size, double value, double *height_map);
	void SampleDiamond(int x, int y, int size, double value, double *height_map);

};

// Might possibly use this struct for hashing XYZ chunk values into a dict for storage and loading
struct XYZHasher {
	std::size_t operator()(const sf::Vector3i& k) const {
		return ((std::hash<int>()(k.x)
			^ (std::hash<int>()(k.y) << 1)) >> 1)
			^ (std::hash<int>()(k.z) << 1);
	}
};

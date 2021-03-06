#pragma once
#include <algorithm>
#include <functional>
#include <random>
#include<SFML/Graphics.hpp>
#include "util.hpp"

class ArrayMap {
	

public:

	ArrayMap(sf::Vector3i dimensions);
	~ArrayMap();

	char getVoxel(sf::Vector3i position);
	void setVoxel(sf::Vector3i position, char value);
	sf::Vector3i getDimensions();

	// =========== DEBUG =========== //
	char* getDataPtr();
	
	std::vector<std::tuple<sf::Vector3i, char>>  CastRayCharArray(
		char* map,
		sf::Vector3i* map_dim,
		sf::Vector2f* cam_dir,
		sf::Vector3f* cam_pos
	);

private:

	char *voxel_data;
	sf::Vector3i dimensions;
	
};

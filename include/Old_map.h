#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <random>
#include <iostream>
#include <functional>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

#include <deque>

class Old_Map {
public:

	Old_Map(sf::Vector3i dim);
	~Old_Map();

	void generate_from_data(char* dat, int len);
	void generate_terrain();

	sf::Vector3i getDimensions();
	char* get_voxel_data();

protected:

private:

	double* height_map;
	char *voxel_data;
	sf::Vector3i dimensions;

	void set_voxel(sf::Vector3i position, int val);
	double sample(int x, int y);
	void set_sample(int x, int y, double value);
	void sample_square(int x, int y, int size, double value);
	void sample_diamond(int x, int y, int size, double value);
	void diamond_square(int stepsize, double scale);


};

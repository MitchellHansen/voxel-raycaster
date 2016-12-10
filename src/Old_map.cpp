#pragma once
#include <iostream>
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include "util.hpp"
#include <Old_map.h>

Old_Map::Old_Map(sf::Vector3i dim) {
	dimensions = dim;
}


Old_Map::~Old_Map() {
}


void Old_Map::generate_terrain() {
	std::mt19937 gen;
	std::uniform_real_distribution<double> dis(-1.0, 1.0);
	auto f_rand = std::bind(dis, std::ref(gen));

	voxel_data = new char[dimensions.x * dimensions.y * dimensions.z];
	height_map = new double[dimensions.x * dimensions.y];

	for (int i = 0; i < dimensions.x * dimensions.y * dimensions.z; i++) {
		voxel_data[i] = 0;
	}

	for (int i = 0; i < dimensions.x * dimensions.y; i++) {
		height_map[i] = 0;
	}

	//size of grid to generate, note this must be a
	//value 2^n+1
	int DATA_SIZE = dimensions.x + 1;
	//an initial seed value for the corners of the data
	//srand(f_rand());
	double SEED = rand() % 25 + 55;

	//seed the data
	set_sample(0, 0, SEED);
	set_sample(0, dimensions.y, SEED);
	set_sample(dimensions.x, 0, SEED);
	set_sample(dimensions.x, dimensions.y, SEED);

	double h = 30.0;//the range (-h -> +h) for the average offset
					//for the new value in range of h
					//side length is distance of a single square side
					//or distance of diagonal in diamond
	for (int sideLength = DATA_SIZE - 1;
	//side length must be >= 2 so we always have
	//a new value (if its 1 we overwrite existing values
	//on the last iteration)
	sideLength >= 2;
		//each iteration we are looking at smaller squares
		//diamonds, and we decrease the variation of the offset
		sideLength /= 2, h /= 2.0) {
		//half the length of the side of a square
		//or distance from diamond center to one corner
		//(just to make calcs below a little clearer)
		int halfSide = sideLength / 2;

		//generate the new square values
		for (int x = 0; x < DATA_SIZE - 1; x += sideLength) {
			for (int y = 0; y < DATA_SIZE - 1; y += sideLength) {
				//x, y is upper left corner of square
				//calculate average of existing corners
				double avg = sample(x, y) + //top left
					sample(x + sideLength, y) +//top right
					sample(x, y + sideLength) + //lower left
					sample(x + sideLength, y + sideLength);//lower right
				avg /= 4.0;

				//center is average plus random offset
				set_sample(x + halfSide, y + halfSide,
					//We calculate random value in range of 2h
					//and then subtract h so the end value is
					//in the range (-h, +h)
					avg + (f_rand() * 2 * h) - h);
			}
		}

		//generate the diamond values
		//since the diamonds are staggered we only move x
		//by half side
		//NOTE: if the data shouldn't wrap then x < DATA_SIZE
		//to generate the far edge values
		for (int x = 0; x < DATA_SIZE - 1; x += halfSide) {
			//and y is x offset by half a side, but moved by
			//the full side length
			//NOTE: if the data shouldn't wrap then y < DATA_SIZE
			//to generate the far edge values
			for (int y = (x + halfSide) % sideLength; y < DATA_SIZE - 1; y += sideLength) {
				//x, y is center of diamond
				//note we must use mod  and add DATA_SIZE for subtraction 
				//so that we can wrap around the array to find the corners
				double avg =
					sample((x - halfSide + DATA_SIZE) % DATA_SIZE, y) + //left of center
					sample((x + halfSide) % DATA_SIZE, y) + //right of center
					sample(x, (y + halfSide) % DATA_SIZE) + //below center
					sample(x, (y - halfSide + DATA_SIZE) % DATA_SIZE); //above center
				avg /= 4.0;

				//new value = average plus random offset
				//We calculate random value in range of 2h
				//and then subtract h so the end value is
				//in the range (-h, +h)
				avg = avg + (f_rand() * 2 * h) - h;
				//update value for center of diamond
				set_sample(x, y, avg);

				//wrap values on the edges, remove
				//this and adjust loop condition above
				//for non-wrapping values.
				if (x == 0)  set_sample(DATA_SIZE - 1, y, avg);
				if (y == 0)  set_sample(x, DATA_SIZE - 1, avg);
			}
		}
	}


	for (int x = 0; x < dimensions.x; x++) {
		for (int y = 0; y < dimensions.y; y++) {

			if (height_map[x + y * dimensions.x] > 0) {
		
				int z = static_cast<int>(height_map[x + y * dimensions.x]);

				while (z > 0) {
					voxel_data[x + dimensions.x * (y + dimensions.z * z)] = 5;
					z--;
				}
			}

		}
	}


	//for (int x = 0; x < dimensions.x / 10; x++) {
	//	for (int y = 0; y < dimensions.y / 10; y++) {
	//		for (int z = 0; z < dimensions.z; z++) {
	//			if (rand() % 1000 < 1)
	//				voxel_data[x + dimensions.x * (y + dimensions.z * z)] = rand() % 6;
	//		}
	//	}
	//}

}


void Old_Map::set_voxel(sf::Vector3i position, int val) {
	voxel_data[position.x + dimensions.x * (position.y + dimensions.z * position.z)] = val;
}

sf::Vector3i Old_Map::getDimensions() {
	return dimensions;
}

char* Old_Map::get_voxel_data() {
	return voxel_data;
}

double Old_Map::sample(int x, int y) {
	return height_map[(x & (dimensions.x - 1)) + (y & (dimensions.y - 1)) * dimensions.x];
}

void Old_Map::set_sample(int x, int y, double value) {
	height_map[(x & (dimensions.x - 1)) + (y & (dimensions.y - 1)) * dimensions.x] = value;
}

void Old_Map::sample_square(int x, int y, int size, double value) {
	int hs = size / 2;

	// a     b 
	//
	//    x
	//
	// c     d

	double a = sample(x - hs, y - hs);
	double b = sample(x + hs, y - hs);
	double c = sample(x - hs, y + hs);
	double d = sample(x + hs, y + hs);

	set_sample(x, y, ((a + b + c + d) / 4.0) + value);

}

void Old_Map::sample_diamond(int x, int y, int size, double value) {
	int hs = size / 2;

	//   c
	//
	//a  x  b
	//
	//   d

	double a = sample(x - hs, y);
	double b = sample(x + hs, y);
	double c = sample(x, y - hs);
	double d = sample(x, y + hs);

	set_sample(x, y, ((a + b + c + d) / 4.0) + value);
}

void Old_Map::diamond_square(int stepsize, double scale) {

	std::mt19937 generator;
	std::uniform_real_distribution<double> uniform_distribution(-1.0, 1.0);
	auto f_rand = std::bind(uniform_distribution, std::ref(generator));

	int halfstep = stepsize / 2;

	for (int y = halfstep; y < dimensions.y + halfstep; y += stepsize) {
		for (int x = halfstep; x < dimensions.x + halfstep; x += stepsize) {
			sample_square(x, y, stepsize, f_rand() * scale);
		}
	}

	for (int y = 0; y < dimensions.y; y += stepsize) {
		for (int x = 0; x < dimensions.x; x += stepsize) {
			sample_diamond(x + halfstep, y, stepsize, f_rand() * scale);
			sample_diamond(x, y + halfstep, stepsize, f_rand() * scale);
		}
	}

}
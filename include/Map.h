#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <random>
#include <iostream>
#include <functional>
#include <cmath>
#include <deque>

class Map {
public: 

	// In addition to traversing the voxel hierarchy, we must also be able to
	// tell which block a given voxel resides in.This is accomplished us -
	// ing 32 - bit page headers spread amongst the child descriptors.Page
	// headers are placed at every 8 kilobyte boundary, and each contains
	// a relative pointer to the block info section.By placing the begin -
	// ning of the child descriptor array at such a boundary, we can always
	// find a page header by simply clearing the lowest bits of any child
	// descriptor pointer.
	struct page_header {
		int bitmask;
	};

	struct leaf_node {
		long bitmask;
	};


	short scale;

	void generate_octree() {

		uint64_t *octree = new uint64_t[200];


		long tree_node = 0;

		std::vector<long> page_array;
		
		// Page placed every 8 kilobytes
		// contains a relative pointer to the block info section
		uint32_t page = 255;
		
		// Child pointer points to the first non-leaf child of this node
		uint16_t child_pointer = 20;


		uint32_t pointer = page | child_pointer;


	};

	Map(sf::Vector3i dim) {

		//generate_octree();
		//return;












		dimensions = dim;
		std::mt19937 gen;
		std::uniform_real_distribution<double> dis(-1.0, 1.0);
		auto f_rand = std::bind(dis, gen);


		list = new char[dim.x * dim.y * dim.z];

		height_map = new double[dim.x * dim.y];

		for (int i = 0; i < dim.x * dim.y; i++) {
			height_map[i] = 0;
		}

		//int featuresize = 2;

		//for (int y = 0; y < dim.y; y += featuresize)
		//	for (int x = 0; x < dim.x; x += featuresize) {
		//		double t = dis(gen);
		//		setSample(x, y, t);  //IMPORTANT: frand() is a random function that returns a value between -1 and 1.
		//	}

		//int samplesize = featuresize;

		//double scale = 10.0;

		//while (samplesize > 1) {

		//	DiamondSquare(samplesize, scale);

		//	samplesize /= 2;
		//	scale /= 2.0;
		//}




		//size of grid to generate, note this must be a
		//value 2^n+1
		int DATA_SIZE = dim.x + 1;
		//an initial seed value for the corners of the data
		double SEED = 50;

		//seed the data
		setSample(0, 0, SEED);
		setSample(0, dim.y, SEED);
		setSample(dim.x, 0, SEED);
		setSample(dim.x, dim.y, SEED);

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
						sample(x + sideLength,y) +//top right
						sample(x,y + sideLength) + //lower left
						sample(x + sideLength,y + sideLength);//lower right
					avg /= 4.0;

					//center is average plus random offset
					setSample(x + halfSide,y + halfSide,
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
						sample((x - halfSide + DATA_SIZE) % DATA_SIZE,y) + //left of center
						sample((x + halfSide) % DATA_SIZE,y) + //right of center
						sample(x,(y + halfSide) % DATA_SIZE) + //below center
						sample(x,(y - halfSide + DATA_SIZE) % DATA_SIZE); //above center
					avg /= 4.0;

					//new value = average plus random offset
					//We calculate random value in range of 2h
					//and then subtract h so the end value is
					//in the range (-h, +h)
					avg = avg + (f_rand() * 2 * h) - h;
					//update value for center of diamond
					setSample(x,y, avg);

					//wrap values on the edges, remove
					//this and adjust loop condition above
					//for non-wrapping values.
					if (x == 0)  setSample(DATA_SIZE - 1,y, avg);
					if (y == 0)  setSample(x, DATA_SIZE - 1, avg);
				}
			}
		}


		for (int x = 0; x < dim.x; x++) {
			for (int y = 0; y < dim.y; y++) {

				if (height_map[x + y * dim.x] > 0) {
					int z = height_map[x + y * dim.x];
					list[x + dim.x * (y + dim.z * z)] = 5;
				}
				
			}
		}


        for (int x = 0; x < dim.x / 10; x++) {
            for (int y = 0; y < dim.y / 10; y++) {
				  for (int z = 0; z < dim.z; z++) {
                    if (rand() % 1000 < 1)
                        list[x + dim.x * (y + dim.z * z)] = rand() % 6;
                }
            }
        }

		
	}

	~Map() {
	}






	sf::Vector3i getDimensions();
	char *list;
	sf::Vector3i dimensions;

	void setVoxel(sf::Vector3i position, int val){

		list[position.x + dimensions.x * (position.y + dimensions.z * position.z)] = val;

	};

	void moveLight(sf::Vector2f in);
	sf::Vector3f global_light;

protected:

private:
	double* height_map;

	double sample(int x, int y) {
		return height_map[(x & (dimensions.x - 1)) + (y & (dimensions.y - 1)) * dimensions.x];
	}

	void setSample(int x, int y, double value) {
		height_map[(x & (dimensions.x - 1)) + (y & (dimensions.y - 1)) * dimensions.x] = value;
	}

	void sampleSquare(int x, int y, int size, double value) {
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

		setSample(x, y, ((a + b + c + d) / 4.0) + value);

	}

	void sampleDiamond(int x, int y, int size, double value) {
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

		setSample(x, y, ((a + b + c + d) / 4.0) + value);
	}

	void DiamondSquare(int stepsize, double scale) {

		std::mt19937 generator;
		std::uniform_real_distribution<double> uniform_distribution(-1.0, 1.0);
		auto f_rand = std::bind(uniform_distribution, std::ref(generator));

		int halfstep = stepsize / 2;

		for (int y = halfstep; y < dimensions.y + halfstep; y += stepsize) {
			for (int x = halfstep; x < dimensions.x + halfstep; x += stepsize) {
				sampleSquare(x, y, stepsize, f_rand() * scale);
			}
		}

		for (int y = 0; y < dimensions.y; y += stepsize) {
			for (int x = 0; x < dimensions.x; x += stepsize) {
				sampleDiamond(x + halfstep, y, stepsize, f_rand() * scale);
				sampleDiamond(x, y + halfstep, stepsize, f_rand() * scale);
			}
		}

	}
};



#include "Map.h"

Map::Map(sf::Vector3i position) {
	
	load_unload(position);
}

int BitCount(unsigned int u) {
	unsigned int uCount;

	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

struct block {
	int header = 0;
	double* data = new double[1000];
};

void Map::generate_octree() {
	
	int* dataset = new int[32 * 32 * 32];
	for (int i = 0; i < 32 * 32 * 32; i++) {
		dataset[0] = i;
	}

	char* arr[8192];
	for (int i = 0; i < 8192; i++) {
		arr[i] = 0;
	}

	std::list<int> parent_stack;

	int byte_pos = 0;

	int levels = log2(32);

	unsigned int parent = 0;
	for (int i = 0; i < 16; i++) {
		parent ^= 1 << i;
	}


	unsigned int leafmask = 255;
	unsigned int validmask = leafmask << 8;
	
	parent &= validmask;
	parent &= leafmask;
	

	std::cout << BitCount(parent & leafmask);

	unsigned int children[8] = {0, 0, 0, 0, 0, 0, 0, 0};


	for (int i = 0; i < levels; i++) {
		
	}




}














//void Map::generate_test() {
//
//	//generate_octree();
//	//return;
//
//	//dimensions = dim;
//	//std::mt19937 gen;
//	//std::uniform_real_distribution<double> dis(-1.0, 1.0);
//	//auto f_rand = std::bind(dis, gen);
//
//
//	//list = new char[dim.x * dim.y * dim.z];
//
//	//height_map = new double[dim.x * dim.y];
//
//	//for (int i = 0; i < dim.x * dim.y * dim.z; i++) {
//	//	list[i] = 0;
//	//}
//
//	//for (int i = 0; i < dim.x * dim.y; i++) {
//	//	height_map[i] = 0;
//	//}
//
//
//	//for (int x = 50; x < 60; x += 2) {
//	//	for (int y = 50; y < 60; y += 2) {
//	//		for (int z = 50; z < 60; z += 2) {
//	//			list[x + dimensions.x * (y + dimensions.z * z)] = 5;
//	//		}
//	//	}
//	//}
//
//	//list[71 + dimensions.x * (61 + dimensions.z * 51)] = 5;
//
//	////for (int x = -dim.x / 2; x < dim.x/2; x++) {
//	////	for (int y = -dim.y / 2; y < dim.y/2; y++) {
//	////		
//	////		double height = 20;
//
//	////		height += std::pow(x / 50.0, 2) - 10 * std::cos(2 * 3.1415926 * x / 50.0);
//	////		height += std::pow(y / 50.0, 2) - 10 * std::cos(2 * 3.1415926 * y / 50.0);
//	////		
//	////		list[(x + dim.x/2) + dim.x * ((y +dim.y/2) + dim.z * (int)height)] = 5;
//	////	}
//	////}
//
//	////int xx = 0;
//	////int yy = 0;
//	////for (int x = -dim.x / 2; x < dim.x / 2; x++) {
//	////	for (int y = -dim.y / 2; y < dim.y / 2; y++) {
//
//	////		double z = 150;
//	//////for (int x = 0; x < dim.x; x++) {
//	//////	for (int y = 0; y < dim.y; y++) {
//	////		double height = 0;
//
//	////		z += -x*2 * std::sin(std::sqrt(abs(x*2 - y*2 - 47))) -
//	////			(y*2 + 47) * std::sin(std::sqrt(std::abs(y*2 + 47 + x*2 / 2)));
//	////	
//
//	////		//z += x * std::sin(std::sqrt(std::abs(y - x + 1))) *
//	////		//	std::cos(std::sqrt(std::abs(y + x + 1))) +
//	////		//	(y + 1) *
//	////		//	std::cos(std::sqrt(std::abs(y - x + 1))) *
//	////		//	std::sin(std::sqrt(std::abs(y + x + 1)));
//	////	
//	////		// Pathological
//	////		//z += 0.5 +
//	////		//	(std::pow(std::sin(std::sqrt(100 * std::pow(x/20, 2) + std::pow(y/20, 2))), 2) - 0.5) /
//	////		//	(1 + 0.001 * std::pow(std::pow(x/20, 2) - 2 * x/20 * y/20 + std::pow(y/20, 2), 2));
//
//	////		// Ackleys
//	////		//z += 20 + M_E -
//	////		//	(20 / (std::pow(M_E, 0.2) * std::sqrt((std::pow(x / 16.0, 2) + std::pow(y / 16.0, 2) + 1) / 2))) -
//	////		//	std::pow(M_E, 0.5 * std::cos(2 * M_PI * x / 16.0) + cos(2 * M_PI * y / 16.0));
//
//	////		//
//	////		//z += -20 * std::pow(M_E, -0.2 * sqrt(0.5 * std::pow(x/64.0, 2) + std::pow(y/64.0, 2))) - std::pow(M_E, 0.5 * (cos(2 * M_PI * x/64.0) + (cos(2 * M_PI * y/64.0)))) + 20 + M_E;
//	////		
//	////		//list[x + dim.x  * (y + dim.z * (int)height)] = 5;
//
//	////		double m = 0.2;
//	////		while ((z*m) > 0){
//	////			list[xx + dim.x * (yy + dim.z * (int)(z*m))] = 5;
//	////			z -= 1/m;
//	////		}
//	////		yy++;
//
//	////	}
//	////	yy = 0;
//	////	xx++;
//	////}
//	////
//
//	////return;
//
//
//
//	////int featuresize = 2;
//
//	////for (int y = 0; y < dim.y; y += featuresize)
//	////	for (int x = 0; x < dim.x; x += featuresize) {
//	////		double t = dis(gen);
//	////		setSample(x, y, t);  //IMPORTANT: frand() is a random function that returns a value between -1 and 1.
//	////	}
//
//	////int samplesize = featuresize;
//
//	////double scale = 10.0;
//
//	////while (samplesize > 1) {
//
//	////	DiamondSquare(samplesize, scale);
//
//	////	samplesize /= 2;
//	////	scale /= 2.0;
//	////}
//
//
//
//
//	////size of grid to generate, note this must be a
//	////value 2^n+1
//	//int DATA_SIZE = dim.x + 1;
//	////an initial seed value for the corners of the data
//	//double SEED = rand() % 25 + 25;
//
//	////seed the data
//	//setSample(0, 0, SEED);
//	//setSample(0, dim.y, SEED);
//	//setSample(dim.x, 0, SEED);
//	//setSample(dim.x, dim.y, SEED);
//
//	//double h = 30.0;//the range (-h -> +h) for the average offset
//	//				//for the new value in range of h
//	//				//side length is distance of a single square side
//	//				//or distance of diagonal in diamond
//	//for (int sideLength = DATA_SIZE - 1;
//	////side length must be >= 2 so we always have
//	////a new value (if its 1 we overwrite existing values
//	////on the last iteration)
//	//sideLength >= 2;
//	//	//each iteration we are looking at smaller squares
//	//	//diamonds, and we decrease the variation of the offset
//	//	sideLength /= 2, h /= 2.0) {
//	//	//half the length of the side of a square
//	//	//or distance from diamond center to one corner
//	//	//(just to make calcs below a little clearer)
//	//	int halfSide = sideLength / 2;
//
//	//	//generate the new square values
//	//	for (int x = 0; x < DATA_SIZE - 1; x += sideLength) {
//	//		for (int y = 0; y < DATA_SIZE - 1; y += sideLength) {
//	//			//x, y is upper left corner of square
//	//			//calculate average of existing corners
//	//			double avg = sample(x, y) + //top left
//	//				sample(x + sideLength, y) +//top right
//	//				sample(x, y + sideLength) + //lower left
//	//				sample(x + sideLength, y + sideLength);//lower right
//	//			avg /= 4.0;
//
//	//			//center is average plus random offset
//	//			setSample(x + halfSide, y + halfSide,
//	//				//We calculate random value in range of 2h
//	//				//and then subtract h so the end value is
//	//				//in the range (-h, +h)
//	//				avg + (f_rand() * 2 * h) - h);
//	//		}
//	//	}
//
//	//	//generate the diamond values
//	//	//since the diamonds are staggered we only move x
//	//	//by half side
//	//	//NOTE: if the data shouldn't wrap then x < DATA_SIZE
//	//	//to generate the far edge values
//	//	for (int x = 0; x < DATA_SIZE - 1; x += halfSide) {
//	//		//and y is x offset by half a side, but moved by
//	//		//the full side length
//	//		//NOTE: if the data shouldn't wrap then y < DATA_SIZE
//	//		//to generate the far edge values
//	//		for (int y = (x + halfSide) % sideLength; y < DATA_SIZE - 1; y += sideLength) {
//	//			//x, y is center of diamond
//	//			//note we must use mod  and add DATA_SIZE for subtraction 
//	//			//so that we can wrap around the array to find the corners
//	//			double avg =
//	//				sample((x - halfSide + DATA_SIZE) % DATA_SIZE, y) + //left of center
//	//				sample((x + halfSide) % DATA_SIZE, y) + //right of center
//	//				sample(x, (y + halfSide) % DATA_SIZE) + //below center
//	//				sample(x, (y - halfSide + DATA_SIZE) % DATA_SIZE); //above center
//	//			avg /= 4.0;
//
//	//			//new value = average plus random offset
//	//			//We calculate random value in range of 2h
//	//			//and then subtract h so the end value is
//	//			//in the range (-h, +h)
//	//			avg = avg + (f_rand() * 2 * h) - h;
//	//			//update value for center of diamond
//	//			setSample(x, y, avg);
//
//	//			//wrap values on the edges, remove
//	//			//this and adjust loop condition above
//	//			//for non-wrapping values.
//	//			if (x == 0)  setSample(DATA_SIZE - 1, y, avg);
//	//			if (y == 0)  setSample(x, DATA_SIZE - 1, avg);
//	//		}
//	//	}
//	//}
//
//
//	//for (int x = 0; x < dim.x; x++) {
//	//	for (int y = 0; y < dim.y; y++) {
//
//	//		if (height_map[x + y * dim.x] > 0) {
//	//			int z = height_map[x + y * dim.x];
//	//			while (z > 0) {
//	//				list[x + dim.x * (y + dim.z * z)] = 5;
//	//				z--;
//	//			}
//	//		}
//
//	//	}
//	//}
//
//
//	//for (int x = 0; x < dim.x / 10; x++) {
//	//	for (int y = 0; y < dim.y / 10; y++) {
//	//		for (int z = 0; z < dim.z; z++) {
//	//			if (rand() % 1000 < 1)
//	//				list[x + dim.x * (y + dim.z * z)] = rand() % 6;
//	//		}
//	//	}
//	//}
//
//}
//
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

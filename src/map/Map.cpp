#include "map/Map.h"



Map::Map(uint32_t dimensions) : array_map(sf::Vector3i(dimensions, dimensions, dimensions)) {

	if ((int)pow(2, (int)log2(dimensions)) != dimensions)
		Logger::log("Map dimensions not an even exponent of 2", Logger::LogLevel::ERROR, __LINE__, __FILE__);

	sf::Vector3i dim3(dimensions, dimensions, dimensions);

	Logger::log("Generating Octree", Logger::LogLevel::INFO);
	octree.Generate(array_map.getDataPtr(), dim3);

	Logger::log("Validating Octree", Logger::LogLevel::INFO);
	if (!octree.Validate(array_map.getDataPtr(), dim3)) {
		Logger::log("Octree validation failed", Logger::LogLevel::ERROR, __LINE__, __FILE__);
	}
}

void Map::setVoxel(sf::Vector3i pos, int val) {
	array_map.getDataPtr()[pos.x + array_map.getDimensions().x * (pos.y + array_map.getDimensions().z * pos.z)] = val;
}

char Map::getVoxel(sf::Vector3i pos){
	
	return array_map.getDataPtr()[pos.x + array_map.getDimensions().x * (pos.y + array_map.getDimensions().z * pos.z)];
	return octree.GetVoxel(pos).found;
}

sf::Vector3f Map::LongRayIntersection(sf::Vector3f origin, sf::Vector3f magnitude) {
	
	sf::Vector3i voxel(origin);

	std::vector<std::tuple<sf::Vector3i, char>> travel_path;

	sf::Vector3f ray_dir(1, 0, 0);

	sf::Vector3i map_dim = array_map.getDimensions();

	// Pitch
	ray_dir = sf::Vector3f(
		ray_dir.z * sin(magnitude.x) + ray_dir.x * cos(magnitude.x),
		ray_dir.y,
		ray_dir.z * cos(magnitude.x) - ray_dir.x * sin(magnitude.x)
	);

	// Yaw
	ray_dir = sf::Vector3f(
		ray_dir.x * cos(magnitude.y) - ray_dir.y * sin(magnitude.y),
		ray_dir.x * sin(magnitude.y) + ray_dir.y * cos(magnitude.y),
		ray_dir.z
	);

	// correct for the base ray pointing to (1, 0, 0) as (0, 0). Should equal (1.57, 0)
	ray_dir = sf::Vector3f(
		static_cast<float>(ray_dir.z * sin(-1.57) + ray_dir.x * cos(-1.57)),
		static_cast<float>(ray_dir.y),
		static_cast<float>(ray_dir.z * cos(-1.57) - ray_dir.x * sin(-1.57))
	);


	// Setup the voxel step based on what direction the ray is pointing
	sf::Vector3i voxel_step(1, 1, 1);

	voxel_step.x *= (ray_dir.x > 0) - (ray_dir.x < 0);
	voxel_step.y *= (ray_dir.y > 0) - (ray_dir.y < 0);
	voxel_step.z *= (ray_dir.z > 0) - (ray_dir.z < 0);


	// Delta T is the units a ray must travel along an axis in order to
	// traverse an integer split
	sf::Vector3f delta_t(
		fabs(1.0f / ray_dir.x),
		fabs(1.0f / ray_dir.y),
		fabs(1.0f / ray_dir.z)
	);

	// offset is how far we are into a voxel, enables sub voxel movement
	// Intersection T is the collection of the next intersection points
	// for all 3 axis XYZ.
	sf::Vector3f intersection_t(
		delta_t.x * (origin.x - floor(origin.x)) * voxel_step.x,
		delta_t.y * (origin.y - floor(origin.y)) * voxel_step.y,
		delta_t.z * (origin.z - floor(origin.z)) * voxel_step.z
	);

	// for negative values, wrap around the delta_t
	intersection_t.x -= delta_t.x * (std::min(intersection_t.x, 0.0f));
	intersection_t.y -= delta_t.y * (std::min(intersection_t.y, 0.0f));
	intersection_t.z -= delta_t.z * (std::min(intersection_t.z, 0.0f));


	int dist = 0;
	sf::Vector3i face_mask(0, 0, 0);
	int voxel_data = 0;

	// Andrew Woo's raycasting algo
	do {

		face_mask.x = intersection_t.x <= std::min(intersection_t.y, intersection_t.z);
		face_mask.y = intersection_t.y <= std::min(intersection_t.z, intersection_t.x);
		face_mask.z = intersection_t.z <= std::min(intersection_t.x, intersection_t.y);

		intersection_t.x += delta_t.x * fabs(face_mask.x);
		intersection_t.y += delta_t.y * fabs(face_mask.y);
		intersection_t.z += delta_t.z * fabs(face_mask.z);

		voxel.x += voxel_step.x * face_mask.x;
		voxel.y += voxel_step.y * face_mask.y;
		voxel.z += voxel_step.z * face_mask.z;

		if (voxel.x >= map_dim.x || voxel.y >= map_dim.y || voxel.z >= map_dim.z) {
			return intersection_t;
		}
		if (voxel.x < 0 || voxel.y < 0 || voxel.z < 0) {
			return intersection_t;
		}

		// If we hit a voxel
		voxel_data = array_map.getDataPtr()[voxel.x + map_dim.x * (voxel.y + map_dim.z * (voxel.z))];

		if (voxel_data != 0)
			return intersection_t;


	} while (++dist < 700.0f);

	return intersection_t;
}

std::vector<sf::Vector3i> Map::BoxIntersection(sf::Vector3f origin, sf::Vector3f magnitude) {
	return std::vector<sf::Vector3i>();
}

sf::Vector3f Map::ShortRayIntersection(sf::Vector3f origin, sf::Vector3f magnitude) {
	return sf::Vector3f(0,0,0);
}

void Map::ApplyHeightmap(sf::Image bitmap) {

}

sf::Image Map::GenerateHeightBitmap(sf::Vector3i dimensions) {

	std::mt19937 gen;
	std::uniform_real_distribution<double> dis(-1.0, 1.0);
	auto f_rand = std::bind(dis, std::ref(gen));

	double* height_map = new double[dimensions.x * dimensions.y];
	for (int i = 0; i < dimensions.x * dimensions.y; i++) {
		height_map[i] = 0;
	}

	//size of grid to generate, note this must be a
	//value 2^n+1
	int DATA_SIZE = dimensions.x + 1;
	//an initial seed value for the corners of the data
	//srand(f_rand());
	double SEED = rand() % 10 + 55;

	//seed the data
	SetSample(0, 0, SEED, height_map);
	SetSample(0, dimensions.y, SEED, height_map);
	SetSample(dimensions.x, 0, SEED, height_map);
	SetSample(dimensions.x, dimensions.y, SEED, height_map);

	double h = 20.0;//the range (-h -> +h) for the average offset
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
				double avg = Sample(x, y, height_map) + //top left
					Sample(x + sideLength, y, height_map) +//top right
					Sample(x, y + sideLength, height_map) + //lower left
					Sample(x + sideLength, y + sideLength, height_map);//lower right
				avg /= 4.0;

				//center is average plus random offset
				SetSample(x + halfSide, y + halfSide,
					//We calculate random value in range of 2h
					//and then subtract h so the end value is
					//in the range (-h, +h)
					avg + (f_rand() * 2 * h) - h, height_map);
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
					Sample((x - halfSide + DATA_SIZE) % DATA_SIZE, y, height_map) + //left of center
					Sample((x + halfSide) % DATA_SIZE, y, height_map) + //right of center
					Sample(x, (y + halfSide) % DATA_SIZE, height_map) + //below center
					Sample(x, (y - halfSide + DATA_SIZE) % DATA_SIZE, height_map); //above center
				avg /= 4.0;

				//new value = average plus random offset
				//We calculate random value in range of 2h
				//and then subtract h so the end value is
				//in the range (-h, +h)
				avg = avg + (f_rand() * 2 * h) - h;
				//update value for center of diamond
				SetSample(x, y, avg, height_map);

				//wrap values on the edges, remove
				//this and adjust loop condition above
				//for non-wrapping values.
				if (x == 0)  SetSample(DATA_SIZE - 1, y, avg, height_map);
				if (y == 0)  SetSample(x, DATA_SIZE - 1, avg, height_map);
			}
		}
	}

	sf::Uint8* pixels = new sf::Uint8[dimensions.x * dimensions.z * 4];

	for (int x = 0; x < dimensions.x; x++) {
		for (int z = 0; z < dimensions.z; z++) {

			sf::Uint8 height = static_cast<sf::Uint8>(std::min(std::max(height_map[x + z * dimensions.x], 0.0), (double)dimensions.z));

			pixels[x + z * dimensions.x * 4 + 0] = height;
			pixels[x + z * dimensions.x * 4 + 1] = height;
			pixels[x + z * dimensions.x * 4 + 2] = height;
			pixels[x + z * dimensions.x * 4 + 3] = sf::Uint8(255);

		}
	}

	sf::Image bitmap_img;
	bitmap_img.create(dimensions.x, dimensions.z, pixels);

	return bitmap_img;

}


double Map::Sample(int x, int y, double *height_map) {
	return height_map[(x & (dimensions.x - 1)) + (y & (dimensions.y - 1)) * dimensions.x];
}

void Map::SetSample(int x, int y, double value, double *height_map) {
	height_map[(x & (dimensions.x - 1)) + (y & (dimensions.y - 1)) * dimensions.x] = value;
}

void Map::SampleSquare(int x, int y, int size, double value, double *height_map) {
	int hs = size / 2;

	double a = Sample(x - hs, y - hs, height_map);
	double b = Sample(x + hs, y - hs, height_map);
	double c = Sample(x - hs, y + hs, height_map);
	double d = Sample(x + hs, y + hs, height_map);

	SetSample(x, y, ((a + b + c + d) / 4.0) + value, height_map);
}

void Map::SampleDiamond(int x, int y, int size, double value, double *height_map) {
	int hs = size / 2;

	double a = Sample(x - hs, y, height_map);
	double b = Sample(x + hs, y, height_map);
	double c = Sample(x, y - hs, height_map);
	double d = Sample(x, y + hs, height_map);

	SetSample(x, y, ((a + b + c + d) / 4.0) + value, height_map);
}

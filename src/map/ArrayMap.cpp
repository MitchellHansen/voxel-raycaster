#include <map/ArrayMap.h>



ArrayMap::ArrayMap(sf::Vector3i dimensions) {
	
	this->dimensions = dimensions;

	voxel_data = new char[dimensions.x * dimensions.y * dimensions.z];
	for (int i = 0; i < dimensions.x * dimensions.y * dimensions.z; i++) {
		voxel_data[i] = 0;
		//voxel_data[i] = 1;
	}

	setVoxel(sf::Vector3i(1, 1, 5), 1);

	for (int x = 0; x < dimensions.x; x++) {
		for (int y = 0; y < dimensions.y; y++) {
			for (int z = 0; z < 1; z++) {
				setVoxel(sf::Vector3i(x, y, z), 5);
			}
		}
	}

// 	for (int x = 0; x < dimensions.x/2-1; x++) {
// 		for (int y = 0; y < dimensions.y/2-1; y++) {
// 			for (int z = 0; z < dimensions.z/2-1; z++) {
// 				setVoxel(sf::Vector3i(x, y, z), 5);
// 			}
// 		}
// 	}
}


ArrayMap::~ArrayMap() {
	delete[] voxel_data;
}

char ArrayMap::getVoxel(sf::Vector3i position) {
	return voxel_data[position.x + dimensions.x * (position.y + dimensions.z * position.z)];
}


void ArrayMap::setVoxel(sf::Vector3i position, char value) {
	voxel_data[position.x + dimensions.x * (position.y + dimensions.z * position.z)] = value;
}

sf::Vector3i ArrayMap::getDimensions() {
	return dimensions;
}

char* ArrayMap::getDataPtr() {
	return voxel_data;
}

std::vector<std::tuple<sf::Vector3i, char>>  ArrayMap::CastRayCharArray(
	char* map,
	sf::Vector3i* map_dim,
	sf::Vector2f* cam_dir,
	sf::Vector3f* cam_pos
) {
	// Setup the voxel coords from the camera origin
	sf::Vector3i voxel(*cam_pos);

	std::vector<std::tuple<sf::Vector3i, char>> travel_path;

	sf::Vector3f ray_dir(1, 0, 0);

	// Pitch
	ray_dir = sf::Vector3f(
		ray_dir.z * sin((*cam_dir).x) + ray_dir.x * cos((*cam_dir).x),
		ray_dir.y,
		ray_dir.z * cos((*cam_dir).x) - ray_dir.x * sin((*cam_dir).x)
	);

	// Yaw
	ray_dir = sf::Vector3f(
		ray_dir.x * cos((*cam_dir).y) - ray_dir.y * sin((*cam_dir).y),
		ray_dir.x * sin((*cam_dir).y) + ray_dir.y * cos((*cam_dir).y),
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
		delta_t.x * (cam_pos->x - floor(cam_pos->x)) * voxel_step.x,
		delta_t.y * (cam_pos->y - floor(cam_pos->y)) * voxel_step.y,
		delta_t.z * (cam_pos->z - floor(cam_pos->z)) * voxel_step.z
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

		if (voxel.x >= map_dim->x || voxel.y >= map_dim->y || voxel.z >= map_dim->z) {
			return travel_path;
		}
		if (voxel.x < 0 || voxel.y < 0 || voxel.z < 0) {
			return travel_path;
		}

		// If we hit a voxel
		voxel_data = map[voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z))];

		travel_path.push_back(std::make_tuple(voxel, voxel_data));

		if (voxel_data != 0)
			return travel_path;


	} while (++dist < 700.0f);

	return travel_path;
}
#include "map/Map.h"


Map::Map(uint32_t dimensions) {


	// ========= TEMP 3D voxel data ===========
	srand(time(nullptr));

	voxel_data = new char[dimensions * dimensions * dimensions];

	for (uint64_t i = 0; i < dimensions * dimensions * dimensions; i++) {

        voxel_data[i] = 1;
    }
	for (uint64_t i = 0; i < dimensions * dimensions * dimensions; i++) {
		if (rand() % 25 < 2)
			voxel_data[i] = 1;
		else
			voxel_data[i] = 0;
	}
	sf::Vector3i dim3(dimensions, dimensions, dimensions);

	octree.Generate(voxel_data, dim3);

    octree.Validate(voxel_data, dim3);

	sf::Vector2f cam_dir(2, 0.01);
	sf::Vector3f cam_pos(10, 10, 10);
	std::vector<std::tuple<sf::Vector3i, char>> list1 = CastRayCharArray(voxel_data, &dim3, &cam_dir, &cam_pos);
	std::vector<std::tuple<sf::Vector3i, char>> list2 = CastRayOctree(&octree, &dim3, &cam_dir, &cam_pos);


	return;

}

void Map::setVoxel(sf::Vector3i pos, int val) {

    voxel_data[pos.x + OCT_DIM * (pos.y + OCT_DIM * pos.z)] = val;
}

char Map::getVoxel(sf::Vector3i pos){

	return octree.GetVoxel(pos).found;
}

std::vector<std::tuple<sf::Vector3i, char>>  Map::CastRayCharArray(
	char* map,
	sf::Vector3i* map_dim,
	sf::Vector2f* cam_dir,
	sf::Vector3f* cam_pos
) {

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


	// Setup the voxel step based on what direction the ray is pointing
	sf::Vector3i voxel_step(1, 1, 1);

	voxel_step.x *= (ray_dir.x > 0) - (ray_dir.x < 0);
	voxel_step.y *= (ray_dir.y > 0) - (ray_dir.y < 0);
	voxel_step.z *= (ray_dir.z > 0) - (ray_dir.z < 0);

	// Setup the voxel coords from the camera origin
	sf::Vector3i voxel(*cam_pos);

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
	intersection_t.x += delta_t.x * -(std::min(intersection_t.x, 0.0f));
	intersection_t.y += delta_t.y * -(std::min(intersection_t.y, 0.0f));
	intersection_t.z += delta_t.z * -(std::min(intersection_t.z, 0.0f));


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

		if ((intersection_t.x) < (intersection_t.y)) {
			if ((intersection_t.x) < (intersection_t.z)) {

				voxel.x += voxel_step.x;
				intersection_t.x = intersection_t.x + delta_t.x;
			}
			else {

				voxel.z += voxel_step.z;
				intersection_t.z = intersection_t.z + delta_t.z;
			}
		}
		else {
			if ((intersection_t.y) < (intersection_t.z)) {

				voxel.y += voxel_step.y;
				intersection_t.y = intersection_t.y + delta_t.y;
			}
			else {

				voxel.z += voxel_step.z;
				intersection_t.z = intersection_t.z + delta_t.z;
			}
		}

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

class Octree;

std::vector<std::tuple<sf::Vector3i, char>> Map::CastRayOctree(
	Octree *octree,
	sf::Vector3i* map_dim,
	sf::Vector2f* cam_dir,
	sf::Vector3f* cam_pos
) {

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


	// Setup the voxel step based on what direction the ray is pointing
	sf::Vector3i voxel_step(1, 1, 1);

	voxel_step.x *= (ray_dir.x > 0) - (ray_dir.x < 0);
	voxel_step.y *= (ray_dir.y > 0) - (ray_dir.y < 0);
	voxel_step.z *= (ray_dir.z > 0) - (ray_dir.z < 0);

	// Setup the voxel coords from the camera origin
	sf::Vector3i voxel(*cam_pos);

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
		delta_t.x * (cam_pos->y - floor(cam_pos->x)) * voxel_step.x,
		delta_t.y * (cam_pos->x - floor(cam_pos->y)) * voxel_step.y,
		delta_t.z * (cam_pos->z - floor(cam_pos->z)) * voxel_step.z
	);

	// for negative values, wrap around the delta_t
	intersection_t.x += delta_t.x * -(std::min(intersection_t.x, 0.0f));
	intersection_t.y += delta_t.y * -(std::min(intersection_t.y, 0.0f));
	intersection_t.z += delta_t.z * -(std::min(intersection_t.z, 0.0f));


	int dist = 0;
	sf::Vector3i face_mask(0, 0, 0);
	int voxel_data = 0;

	OctState traversal_state = octree->GetVoxel(voxel);

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

		if ((intersection_t.x) < (intersection_t.y)) {
			if ((intersection_t.x) < (intersection_t.z)) {

				voxel.x += voxel_step.x;
				intersection_t.x = intersection_t.x + delta_t.x;
			}
			else {

				voxel.z += voxel_step.z;
				intersection_t.z = intersection_t.z + delta_t.z;
			}
		}
		else {
			if ((intersection_t.y) < (intersection_t.z)) {

				voxel.y += voxel_step.y;
				intersection_t.y = intersection_t.y + delta_t.y;
			}
			else {

				voxel.z += voxel_step.z;
				intersection_t.z = intersection_t.z + delta_t.z;
			}
		}

		if (voxel.x >= map_dim->x || voxel.y >= map_dim->y || voxel.z >= map_dim->z) {
			return travel_path;
		}
		if (voxel.x < 0 || voxel.y < 0 || voxel.z < 0) {
			return travel_path;
		}

		// If we hit a voxel
		//voxel_data = map[voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z))];
		voxel_data = getVoxel(voxel);
		travel_path.push_back(std::make_tuple(voxel, voxel_data));

		if (voxel_data != 0)
			return travel_path;


	} while (++dist < 700.0f);

	return travel_path;
}
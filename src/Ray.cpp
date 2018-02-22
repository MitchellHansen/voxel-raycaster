#include <Ray.h>

Ray::Ray(
        Map *map,
        sf::Vector2<int> resolution,
        sf::Vector2<int> pixel,
        sf::Vector3<float> camera_position,
        sf::Vector3<float> ray_direction) {

    this->pixel = pixel;
    this->map = map;
    origin = camera_position;
    direction = ray_direction;

	// TODO: Had to break this while refactoring map
	dimensions = sf::Vector3i(0, 0, 0); // map->getDimensions();
}

sf::Color Ray::Cast() {

    // Setup the voxel step based on what direction the ray is pointing
    sf::Vector3<int> voxel_step(1, 1, 1);
    voxel_step.x *= (direction.x > 0) - (direction.x < 0);
    voxel_step.y *= (direction.y > 0) - (direction.y < 0);
    voxel_step.z *= (direction.z > 0) - (direction.z < 0);

    // Setup the voxel coords from the camera origin
    voxel = sf::Vector3<int>(
        static_cast<int>(floorf(origin.x)),
		static_cast<int>(floorf(origin.y)),
		static_cast<int>(floorf(origin.z))
    );

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
    delta_t = sf::Vector3<float>(
        fabsf(1.0f / direction.x),
        fabsf(1.0f / direction.y),
        fabsf(1.0f / direction.z)
    );

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
    intersection_t = sf::Vector3<float>(
        delta_t.x,
        delta_t.y,
        delta_t.z
    );


    int dist = 0;
    int face = -1;
    // X:0, Y:1, Z:2

    // Andrew Woo's raycasting algo
    do {
        if ((intersection_t.x) < (intersection_t.y)) {
            if ((intersection_t.x) < (intersection_t.z)) {

                face = 0;
                voxel.x += voxel_step.x;
                intersection_t.x = intersection_t.x + delta_t.x;
            } else {

                face = 2;
                voxel.z += voxel_step.z;
                intersection_t.z = intersection_t.z + delta_t.z;
            }
        } else {
            if ((intersection_t.y) < (intersection_t.z)) {

                face = 1;
                voxel.y += voxel_step.y;
                intersection_t.y = intersection_t.y + delta_t.y;
            } else {

                face = 2;
                voxel.z += voxel_step.z;
                intersection_t.z = intersection_t.z + delta_t.z;
            }
        }

        // If the ray went out of bounds
        if (voxel.z >= dimensions.z) {
            return sf::Color(172, 245, 251, 200);
        }
        if (voxel.x >= dimensions.x) {
			return sf::Color(172, 245, 251, 200);
        }
        if (voxel.y >= dimensions.x) {
			return sf::Color(172, 245, 251, 200);
        }

        if (voxel.x < 0) {
			return sf::Color(172, 245, 251, 200);
        }
        if (voxel.y < 0) {
			return sf::Color(172, 245, 251, 200);
        }
        if (voxel.z < 0) {
			return sf::Color(172, 245, 251, 200);
        }

        // If we hit a voxel
        int index = voxel.x + dimensions.x * (voxel.y + dimensions.z * voxel.z);
        
    	// TODO: Had to break this while refactoring map
		int voxel_data = 0; // map->list[index];

        float alpha = 0;
        if (face == 0) {

            //alpha = AngleBetweenVectors(sf::Vector3f(1, 0, 0), map->global_light);
            alpha = static_cast<float>(fmod(alpha, 0.785) * 2);

        } else if (face == 1) {

            //alpha = AngleBetweenVectors(sf::Vector3f(0, 1, 0), map->global_light);
            alpha = static_cast<float>(fmod(alpha, 0.785) * 2);

        } else if (face == 2){

            //alpha = 1.57 / 2;
            //alpha = AngleBetweenVectors(sf::Vector3f(0, 0, 1), map->global_light);
            alpha = static_cast<float>(fmod(alpha, 0.785) * 2);
        }

        alpha *= 162;

        switch (voxel_data) {
            case 5:
                return sf::Color(255, 120, 255, static_cast<int>(alpha));
            case 6:
                return sf::Color(150, 80, 220, static_cast<int>(alpha));
			default:
				return sf::Color(150, 80, 220, static_cast<int>(alpha));
        }

        dist++;


    } while (dist < 600);

    // Ray timeout color
    return sf::Color::Cyan;
}




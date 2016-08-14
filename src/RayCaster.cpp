#include "RayCaster.h"
#include <util.hpp>
#include <Ray.h>


RayCaster::RayCaster(
	Map *map,
	sf::Vector3<int> map_dimensions,
    sf::Vector2<int> viewport_resolution ) {

	// Override values
	//this.map_dimensions = new Vector3<int> (50, 50, 50);
	//this.resolution = new Vector2<int> (200, 200);
	//this.camera_direction = new Vector3<float> (1f, 0f, .8f);
	//this.camera_position = new Vector3<float> (1, 10, 10);

	this->map_dimensions = map_dimensions;
	this->map = map;

	resolution = viewport_resolution;
	image = new sf::Color[resolution.x * resolution.y];


    // Calculate the view plane vectors
    // Because casting to individual pixels causes barrel distortion,
    // Get the radian increments
    // Set the camera origin
    // Rotate the ray by the specified pixel * increment

    double y_increment_radians = DegreesToRadians(50.0 / resolution.y);
    double x_increment_radians = DegreesToRadians(80.0 / resolution.x);

    view_plane_vectors = new sf::Vector3f[resolution.x * resolution.y];
    for (int y = -resolution.y / 2 ; y < resolution.y / 2; y++) {
        for (int x = -resolution.x / 2; x < resolution.x / 2; x++) {

            // The base ray direction to slew from
            sf::Vector3f ray(1, 0, 0);

            // Y axis, pitch
            ray = sf::Vector3f(
                    ray.z * sin(y_increment_radians * y) + ray.x * cos(y_increment_radians * y),
                    ray.y,
                    ray.z * cos(y_increment_radians * y) - ray.x * sin(y_increment_radians * y)
            );

            // Z axis, yaw
            ray = sf::Vector3f(
                    ray.x * cos(x_increment_radians * x) - ray.y * sin(x_increment_radians * x),
                    ray.x * sin(x_increment_radians * x) + ray.y * cos(x_increment_radians * x),
                    ray.z
            );

            int index = (x + resolution.x / 2) + resolution.x * (y + resolution.y / 2);
            view_plane_vectors[index] = Normalize(ray);
        }
    }
}

RayCaster::~RayCaster() {
    delete image;
    delete view_plane_vectors;
}

sf::Color* RayCaster::CastRays(sf::Vector3<float> camera_direction, sf::Vector3<float> camera_position) {

    // Setup the camera for this cast
    this->camera_direction = camera_direction;
    camera_direction_cartesian = Normalize(SphereToCart(camera_direction));
    this->camera_position = camera_position;

    // Start the loop at the top left, scan right and work down
    for (int y = 0; y < resolution.y; y++) {
        for (int x = 0; x < resolution.x; x++) {

            // Get the ray at the base direction
            sf::Vector3f ray = view_plane_vectors[x + resolution.x * y];

            // Rotate it to the correct pitch and yaw

            // Y axis, pitch
            ray = sf::Vector3f(
                    ray.z * sin(camera_direction.y) + ray.x * cos(camera_direction.y),
                    ray.y,
                    ray.z * cos(camera_direction.y) - ray.x * sin(camera_direction.y)
            );

            // Z axis, yaw
            ray = sf::Vector3f(
                    ray.x * cos(camera_direction.z) - ray.y * sin(camera_direction.z),
                    ray.x * sin(camera_direction.z) + ray.y * cos(camera_direction.z),
                    ray.z
            );


            // Setup the ray
            Ray r(map, resolution, sf::Vector2i(x, y), camera_position, ray);

            // Cast it and assign its return value
            image[x + resolution.x * y] = r.Cast();
        }
    }

    return image;
}

void RayCaster::moveCamera(sf::Vector2f v) {
	camera_direction.y += v.x;
	camera_direction.z += v.y;
}

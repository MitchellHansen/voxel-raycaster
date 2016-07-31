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



}


RayCaster::~RayCaster() {
}



sf::Color* RayCaster::CastRays(sf::Vector3<float> camera_direction, sf::Vector3<float> camera_position) {

		// Setup the camera for this cast
		this->camera_direction = camera_direction;
		camera_direction_cartesian = Normalize(SphereToCart(camera_direction));
		this->camera_position = camera_position;


		// The radian increment each ray is spaced from one another
		double y_increment_radians = DegreesToRadians(40.0 / resolution.y);
		double x_increment_radians = DegreesToRadians(50.0 / resolution.x);


		// A reference to the positive X axis as our base viewport point
		sf::Vector3f base_direction(1, 0, 0);
		
		//-resolution.y / 2
		// Start the loop at the top left, scan right and work down
		for (int x = 0; x < resolution.x; x++) {
			for (int y = 0; y < resolution.y; y++) {

				// The direction the final ray will point.
				// First take a reference to the base direction to setup the viewport
				//Vector3<float> ray_direction = new Vector3<float> (base_direction);

				// New method to cast rays using the original intended Spherical coords
				// instead of that malarchy with converting them to cartesian from the formula


				sf::Vector3f ray_direction(
					camera_direction.x,
					camera_direction.y + (float)(y_increment_radians * y),
					camera_direction.z + (float)(x_increment_radians * x)
				);

				sf::Vector3f ray_cartesian = Normalize(SphereToCart(ray_direction));
				sf::Vector3f cam_cartesian = Normalize(SphereToCart(camera_direction));

				if ((y == -99 || y == 0 || y == 99) && (/*x == 99 || x == 0 || */x == -99)) {
					std::cout << "X : " << x << "\n";
					std::cout << "Y : " << y << "\n";
					std::cout << ray_direction.x << " : " << ray_direction.y << " : " << ray_direction.z << "\n";
				}

				// Setup the ray
				Ray r(map, resolution, sf::Vector2i(x, y), camera_position, ray_direction);

				// Cast it
				sf::Color c = r.Cast();
				if (c.a == 0)
					std::cout << "BLACK";
				image[x + resolution.x*y] = c;
			}
		}


		return image;
	}

void RayCaster::moveCamera(sf::Vector2f v) {
	camera_direction.y += v.x;
	camera_direction.z += v.y;
}

#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <Map.h>

class RayCaster {
public:
	RayCaster(Map *map,
			 sf::Vector3<int> map_dimensions,
			 sf::Vector2<int> viewport_resolution);
	~RayCaster();


	sf::Color* CastRays(sf::Vector3<float> camera_direction, sf::Vector3<float> camera_position);
	void moveCamera(sf::Vector2f v);
private:

	sf::Vector3<int> map_dimensions;
	Map *map;

	// The XY resolution of the viewport
	sf::Vector2<int> resolution;

	// The pixel array, maybe do RBGA? Are there even 4 byte data types?
	sf::Color *image;

	// The direction of the camera in POLAR coordinates
	sf::Vector3<float> camera_direction;


	// Convert the polar coordinates to CARTESIAN
	sf::Vector3<float> camera_direction_cartesian;

	// The world-space position of the camera
	sf::Vector3<float> camera_position;
};


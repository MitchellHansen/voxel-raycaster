#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <Map.h>
#include "Old_map.h"


// What about a parent, child relationship between the raycaster and it's two
// different modes? Raycaster -> ClCaster, SoftwareCaster
class Camera;

class RayCaster {
public:

	RayCaster();
	~RayCaster();

	virtual void assign_map(Old_Map *map) = 0;
	virtual void assign_camera(Camera *camera) = 0;
	virtual void assign_viewport(int width, int height, float v_fov, float h_fov) = 0;
	virtual void assign_light(Light light) = 0;

	// draw will abstract the gl sharing and software rendering
	// methods of retrieving the screen buffer
	virtual void draw(sf::RenderWindow* window) = 0;

private:

	sf::Vector3<int> map_dimensions;
	Old_Map *map;

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

    // The distance in units the view plane is from the iris point
    int view_plane_distance = 200;

    // Precalculated values for the view plane rays
    sf::Vector3f *view_plane_vectors;

};


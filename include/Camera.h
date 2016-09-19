#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include "util.hpp"

class Camera {
public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	Camera();
	Camera(sf::Vector3f position, sf::Vector2f direction);
	~Camera();

	int set_position(sf::Vector3f position);

	int add_static_impulse(sf::Vector3f impulse);
	int add_relative_impulse(DIRECTION direction);

	int slew_camera(sf::Vector2f input);

	int update();

	void* get_direction_pointer();
	void* get_position_pointer();

	sf::Vector3f get_movement();
	sf::Vector3f get_position();
	sf::Vector2f get_direction();

private:

	float friction_coefficient = 0.1;
	float default_impulse = 1.0;

	// 3D vector
	sf::Vector3f movement;

	// XYZ
	sf::Vector3f position;

	// These are spherical coords
	sf::Vector2f direction;
};


#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp> 
#include "util.hpp"
#include "Pub_Sub.h"

class Camera : public VrEventSubscriber{
public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	Camera();
	Camera(sf::Vector3f position, sf::Vector2f direction, sf::RenderWindow *window);
	~Camera();

	int set_position(sf::Vector3f position);

	int add_static_impulse(sf::Vector3f impulse);
	int add_relative_impulse(DIRECTION direction, float speed);

	int slew_camera(sf::Vector2f input);
	void set_camera(sf::Vector2f input);
	void set_camera(sf::Vector3f input);

	int update(double delta_time);

	void look_at_center();

	sf::Vector2f* get_direction_pointer();
	sf::Vector3f* get_position_pointer();
	sf::Vector3f* get_movement_pointer();

	sf::Vector3f get_movement();
	sf::Vector3f get_position();
	sf::Vector2f get_direction();

	void setSpeed(float speed);
	float getSpeed();

	void recieve_event(VrEventPublisher* publisher, std::unique_ptr<vr::Event> event) override;

private:

	float friction_coefficient = 0.1f;
	float default_impulse = 1.0f;

	// 3D vector
	sf::Vector3f movement;

	// XYZ
	sf::Vector3f position;

	// These are spherical coords
	sf::Vector2f direction;

	bool mouse_enabled = true;
	sf::Vector2i deltas;
	sf::Vector2i fixed;
	sf::Vector2i prev_pos;

	sf::RenderWindow *window;
};


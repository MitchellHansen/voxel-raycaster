#pragma once
#include <cmath>
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include "Gui.h"
#include "Pub_Sub.h"
#include "util.hpp"



/**
 *
 * Camera
 *
 * Camera provides a convenient way to create 3d vectors and positions which represent a camera. It provides physics
 * to move the camera around in 3d space as well as impulse and friction control to alter its movement characteristics
 *
 */


class Camera : public VrEventSubscriber, private Gui{

public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	Camera();
    // TODO: Remove dependency on getting a window ptr. Instead provide window interface to get and set mouse position
	Camera(sf::Vector3f position, sf::Vector2f direction, sf::RenderWindow *window);
	~Camera();

	int set_position(sf::Vector3f position);

    // Apply an incoming impule to the velocity vector
	int add_static_impulse(sf::Vector3f impulse);

    // Apply an impulse in one of the 6 relative directions with a certain magnitude
	int add_relative_impulse(DIRECTION direction, float speed);

	int slew_camera(sf::Vector2f input);
	void set_camera_direction(sf::Vector2f input);
	void set_camera_direction(sf::Vector3f input);

	int update(double delta_time);

	void look_at_center();

    // TODO: Raw ptr's SHARED_PTR with CL, bad idea
	sf::Vector2f* get_direction_pointer();
	sf::Vector3f* get_position_pointer();
	sf::Vector3f* get_movement_pointer();

	sf::Vector3f get_movement();
	sf::Vector3f get_position();
	sf::Vector2f get_direction();

	void setSpeed(float speed);
	float getSpeed();

	void event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event> event) override;


	virtual void render_gui() override;
	virtual void update_gui() override;

private:

	float friction_coefficient = 0.001f;
	float default_impulse = 0.1f;

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


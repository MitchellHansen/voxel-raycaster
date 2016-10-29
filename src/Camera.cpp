#include "Camera.h"



Camera::Camera() {
}


Camera::Camera(sf::Vector3f position, sf::Vector2f direction) :
	position(position), direction(direction)
	{

}

Camera::~Camera() {
}

int Camera::set_position(sf::Vector3f position) {
	this->position = position;
	return 1;
}
	
int Camera::add_static_impulse(sf::Vector3f impulse) {
	movement += impulse;
	return 1;
}

int Camera::add_relative_impulse(DIRECTION impulse_direction, float speed) {
	
	// No sense in doing fancy dot products, adding Pi's will suffice
	// Always add PI/2 to X initially to avoid negative case
	sf::Vector2f dir;

	switch (impulse_direction) {
	
	case DIRECTION::UP:
		dir = sf::Vector2f(direction.y, direction.x + PI_F);
		break;
	case DIRECTION::DOWN:
		dir = sf::Vector2f(direction.y, direction.x);
		break;
	case DIRECTION::LEFT:
		dir = sf::Vector2f(direction.y + PI_F + PI_F / 2, PI_F / 2);
		break;
	case DIRECTION::RIGHT:
		dir = sf::Vector2f(direction.y + PI_F / 2, PI_F / 2);
		break;
	case DIRECTION::FORWARD:
		dir = sf::Vector2f(direction.y, direction.x + PI_F / 2);
		break;
	case DIRECTION::REARWARD:
		dir = sf::Vector2f(direction.y + PI_F, (direction.x * -1) + PI_F / 2 );
		break;

	}

	movement += SphereToCart(dir);
	movement *= speed;

	return 1;
}

int Camera::slew_camera(sf::Vector2f input) {
	direction -= input;
	return 1;
}

int Camera::update(double delta_time) {
	
	// so vector multiplication broke?
	// have to do it component wise
	double multiplier = 40;

	position.x += movement.x * delta_time * multiplier;
	position.y += movement.y * delta_time * multiplier;
	position.z += movement.z * delta_time * multiplier;
	
	movement *= (float)(friction_coefficient * delta_time * multiplier);
	
	return 1;
}

sf::Vector2f* Camera::get_direction_pointer() {
	return &direction;
}

sf::Vector3f* Camera::get_movement_pointer() {
	return &movement;
}

sf::Vector3f* Camera::get_position_pointer() {
	return &position;
}

sf::Vector3f Camera::get_movement() {
	return movement;
}

sf::Vector3f Camera::get_position() {
	return position;
}

sf::Vector2f Camera::get_direction() {
	return direction;
}
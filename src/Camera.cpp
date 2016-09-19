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

int Camera::add_relative_impulse(DIRECTION impulse_direction) {

	SphereToCart(direction);

	return 1;
}

int Camera::slew_camera(sf::Vector2f input) {
	direction -= input;
	return 1;
}

int Camera::update() {
	
	position += movement;
	
	movement *= friction_coefficient;
	
	return 1;
}

void* Camera::get_direction_pointer() {
	return &direction;
}

void* Camera::get_position_pointer() {
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
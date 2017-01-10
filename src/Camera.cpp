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
	
	case DIRECTION::FORWARD:
		dir = sf::Vector2f(direction.y, direction.x);
		break;
	case DIRECTION::REARWARD:
		dir = sf::Vector2f(direction.y, direction.x + PI_F);
		break;
	case DIRECTION::LEFT:
		dir = sf::Vector2f(direction.y + PI_F + PI_F / 2, PI_F / 2);
		break;
	case DIRECTION::RIGHT:
		dir = sf::Vector2f(direction.y + PI_F / 2, PI_F / 2);
		break;
	case DIRECTION::UP:
		dir = sf::Vector2f(direction.y, direction.x + PI_F / 2);
		break;
	case DIRECTION::DOWN:
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

void Camera::set_camera(sf::Vector2f input) {
	direction = input;
}

void Camera::set_camera(sf::Vector3f input) {
	direction = CartToNormalizedSphere(input);
}

int Camera::update(double delta_time) {
	
	double multiplier = 40;

	position.x += static_cast<float>(movement.x * delta_time * multiplier);
	position.y += static_cast<float>(movement.y * delta_time * multiplier);
	position.z += static_cast<float>(movement.z * delta_time * multiplier);
	
	movement *= static_cast<float>(friction_coefficient * delta_time * multiplier);
	
	return 1;
}

void Camera::update(SfEventPublisher* p, sf::Event e)
{

}

void Camera::look_at_center() {

	direction = CartToNormalizedSphere(sf::Vector3f(75, 75, 75) - position);

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
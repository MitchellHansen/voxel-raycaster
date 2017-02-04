#pragma once
#include "LightController.h"
#include "Pub_Sub.h"

LightController::LightController(std::shared_ptr<RayCaster> raycaster) : 
	raycaster(raycaster) {
	
	
	
	//packed_index = packed_data.size() / packed_size;
}

LightController::~LightController() {
}

void LightController::set_position(sf::Vector3f position) {

}

int LightController::add_static_impulse(sf::Vector3f impulse) {
	return 1;
}

int LightController::add_relative_impulse(DIRECTION impulse_direction, float speed) {

	// No sense in doing fancy dot products, adding Pi's will suffice
	// Always add PI/2 to X initially to avoid negative case
	sf::Vector2f dir;

	switch (impulse_direction) {

	case DIRECTION::FORWARD:
		dir = sf::Vector2f(packed_data_array.at(packed_index).direction_cartesian.y, packed_data_array.at(packed_index).direction_cartesian.x);
		break;
	case DIRECTION::REARWARD:
		dir = sf::Vector2f(packed_data_array.at(packed_index).direction_cartesian.y, packed_data_array.at(packed_index).direction_cartesian.x + PI_F);
		break;
	case DIRECTION::LEFT:
		dir = sf::Vector2f(packed_data_array.at(packed_index).direction_cartesian.y + PI_F + PI_F / 2, PI_F / 2);
		break;
	case DIRECTION::RIGHT:
		dir = sf::Vector2f(packed_data_array.at(packed_index).direction_cartesian.y + PI_F / 2, PI_F / 2);
		break;
	case DIRECTION::UP:
		dir = sf::Vector2f(packed_data_array.at(packed_index).direction_cartesian.y, packed_data_array.at(packed_index).direction_cartesian.x + PI_F / 2);
		break;
	case DIRECTION::DOWN:
		dir = sf::Vector2f(packed_data_array.at(packed_index).direction_cartesian.y + PI_F, (packed_data_array.at(packed_index).direction_cartesian.x * -1) + PI_F / 2);
		break;

	}

	//movement += SphereToCart(dir);
	//movement *= speed;

	return 1;
}


int LightController::update(double delta_time) {

	double multiplier = 40;

	//position.x += static_cast<float>(movement.x * delta_time * multiplier);
	//position.y += static_cast<float>(movement.y * delta_time * multiplier);
	//position.z += static_cast<float>(movement.z * delta_time * multiplier);

	//movement *= static_cast<float>(1.0f * delta_time * multiplier);

	return 1;
}

void LightController::recieve_event(VrEventPublisher* publisher, std::unique_ptr<vr::Event> event) {

	if (event.get()->type == vr::Event::KeyHeld) {}
	else if (event->type == vr::Event::KeyPressed) {}
	else if (event->type == vr::Event::MouseMoved) {}

	else if (event->type == vr::Event::JoystickMoved) {

		vr::JoystickMoved *joystick_event = static_cast<vr::JoystickMoved*>(event.get());

		if (joystick_event->axis == sf::Joystick::Axis::X) {
			//movement.x -= joystick_event->position / 5;
			//add_relative_impulse(Camera::DIRECTION::FORWARD, joystick_event->position);
		}
		else if (joystick_event->axis == sf::Joystick::Axis::Y) {
			//movement.y += joystick_event->position / 5;
			//add_relative_impulse(Camera::DIRECTION::RIGHT, joystick_event->position);
		}
		//else if (joystick_event->axis == sf::Joystick::Axis::Z) {
		//	add_relative_impulse(Camera::DIRECTION::DOWN, joystick_event->position);
		//}
	}

}

void LightController::erase_light() {
	//packed_data.emplace_back(PackedData(position, direction, rgbi));
}

std::vector<LightController::PackedData>* LightController::get_lights() {
	return &packed_data;
}

void LightController::look_at_center() {

	//direction_cartesian = CartToNormalizedSphere(sf::Vector3f(75, 75, 75) - position);
}


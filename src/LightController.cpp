#pragma once
#include "LightController.h"

//LightController::LightController(std::shared_ptr<RayCaster> raycaster) {
//	//:raycaster(raycaster) {
//	
//	
//	
//	//packed_index = packed_data.size() / packed_size;
//}

LightController::~LightController() {
}

//void LightController::create_light(LightController::PackedData light_data, std::string light_name) {
//	
//	//if (light_map.count(light_name) == 1) {
//	//	// light already exists, TODO: error out
//	//	return;
//	//}
//
//	
//}

//LightHandle LightController::get_light_handle(std::string light_name) {

//}

void LightController::set_position(sf::Vector3f position) {

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

//std::vector<LightController::PackedData>* LightController::get_lights() {
//	return &packed_data_array;
//}

void LightController::look_at_center() {

	//direction_cartesian = CartToNormalizedSphere(sf::Vector3f(75, 75, 75) - position);
}


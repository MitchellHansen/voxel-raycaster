#include "LightController.h"

LightController::LightController(std::shared_ptr<Hardware_Caster> raycaster) : packed_data_array(reserved_count), open_list(reserved_count) {

	std::iota(open_list.begin(), open_list.end(), 0);

	raycaster->assign_lights(&packed_data_array);
	
}

LightController::~LightController() {
	
}


std::shared_ptr<LightHandle> LightController::create_light(LightPrototype light_prototype) {

	unsigned int index = open_list.front();
	open_list.pop_front();
	
	PackedData* data = &packed_data_array.data()[index];

	std::shared_ptr<LightHandle> handle(new LightHandle(this, index, light_prototype, data));
	
	return handle;

}


void LightController::remove_light(unsigned int light_index) {

	// Sanitization is currently handled by the light handler	
	open_list.push_front(light_index);

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

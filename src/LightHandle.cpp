#include "LightHandle.h"
#include "LightController.h"


LightHandle::LightHandle(LightController *const light_controller, unsigned int light_id, LightPrototype light_prototype, PackedData *const data_reference) :
	light_controller_ref(light_controller), light_id(light_id), data_reference(data_reference) {

	friction_coefficient = light_prototype.friction;
	default_impulse = light_prototype.impulse;
	movement = light_prototype.movement;

	data_reference->direction_cartesian = light_prototype.direction_cartesian;
	data_reference->position = light_prototype.position;
	data_reference->rgbi = light_prototype.rgbi;

}


LightHandle::~LightHandle() {

	// Sanitize data here, or in the controller?
	data_reference->direction_cartesian = sf::Vector3f(0, 0, 0);
	data_reference->position = sf::Vector3f(0, 0, 0);
	data_reference->rgbi = sf::Vector4f(0, 0, 0, 0);

	light_controller_ref->remove_light(light_id);

}


void LightHandle::set_friction(float friction)
{

}

void LightHandle::set_impulse(float impulse)
{

}

void LightHandle::set_movement(sf::Vector3f movement)
{

}

void LightHandle::add_movement(sf::Vector3f movement)
{

}

void LightHandle::set_position(sf::Vector3f position)
{
	data_reference->position = position;
}


sf::Vector3f LightHandle::get_position() {
	return data_reference->position;
}

void LightHandle::set_direction(sf::Vector3f direction)
{

}

void LightHandle::set_rgbi(sf::Vector4f rgbi)
{
	data_reference->rgbi = rgbi;
}

void LightHandle::event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event> event)
{
		if (event->type == vr::Event::JoystickMoved) {
			
			vr::JoystickMoved *joystick_event = static_cast<vr::JoystickMoved*>(event.get());

			if (joystick_event->axis == sf::Joystick::Axis::X) {
				movement.x = -joystick_event->position / 100;
				//add_relative_impulse(Camera::DIRECTION::FORWARD, joystick_event->position);
			}
			else if (joystick_event->axis == sf::Joystick::Axis::Y) {
				movement.y = joystick_event->position / 100;
				//add_relative_impulse(Camera::DIRECTION::RIGHT, joystick_event->position);
			}
			else if (joystick_event->axis == sf::Joystick::Axis::Z) {
				movement.y = joystick_event->position / 100;
			}
		}
}

void LightHandle::update(double delta_time) {
	
	double multiplier = 40;

	data_reference->position.x += static_cast<float>(movement.x * delta_time * multiplier);
	data_reference->position.y += static_cast<float>(movement.y * delta_time * multiplier);
	data_reference->position.z += static_cast<float>(movement.z * delta_time * multiplier);

	//movement *= static_cast<float>(friction_coefficient * delta_time * multiplier);

}

void LightHandle::render_gui() {
	ImGui::Begin("Lights");

	// Well I'm noooot reaallly supposed to do this. But I've been doing with the caster for
	// a wile with no problems....
	ImGui::SliderFloat4("Color", reinterpret_cast<float*>(&data_reference->rgbi), 0, 1);
	ImGui::SliderFloat3("Position", reinterpret_cast<float*>(&data_reference->position), 0, 256);

	ImGui::Separator();
	
	ImGui::End();
}

void LightHandle::update_gui() {
	rendering = true;
}


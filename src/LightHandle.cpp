#include "LightHandle.h"


LightHandle::LightHandle() {

	// init an empty light
	LightController::PackedData data;
	data.direction_cartesian = sf::Vector3f(0, 0, 0);
	data.position = sf::Vector3f(0, 0, 0);
	data.rgbi = sf::Vector4f(0, 0, 0, 0);
	
	//light_controller.create_light(data, light_name);
}

LightHandle::~LightHandle() {

}


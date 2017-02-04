#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp> 
#include "util.hpp"
#include "Pub_Sub.h"
#include "RayCaster.h"

class LightController : public VrEventSubscriber {
public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	// Packed data structure for passing raw light data to the caster
	struct PackedData {
		PackedData(sf::Vector3f position, sf::Vector3f direction_cartesian, sf::Vector4f rgbi) :
			position(position), direction_cartesian(direction_cartesian), rgbi(rgbi) {
		}
		PackedData();
		sf::Vector3f position;
		sf::Vector3f direction_cartesian;
		sf::Vector4f rgbi;
	};

	// Data that enables "camera" style movement. I can't really use inheritance easily because
	// of the data packing requirements
	struct Light {
		Light();
		int packed_index;
		float friction_coefficient = 0.1f;
		float default_impulse = 1.0f;
		sf::Vector3f movement;
	};

	LightController(std::shared_ptr<RayCaster> raycaster);
	~LightController();

	void set_position(sf::Vector3f position);

	int add_static_impulse(sf::Vector3f impulse);
	int add_relative_impulse(DIRECTION direction, float speed);

	int update(double delta_time);

	void look_at_center();

	void recieve_event(VrEventPublisher* p, std::unique_ptr<vr::Event> event) override;

	void erase_light();
	std::vector<LightController::PackedData>* get_lights();
private:


	// Need to allow N byte light class to be packed into 10 byte packets
	int packed_size = sizeof(PackedData);
	
	// Index that this light is in the packed data
	int packed_index;

	std::vector<PackedData> packed_data_array;
	std::unordered_map<std::string, Light> light_map;
	std::shared_ptr<RayCaster> raycaster;


};


#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp> 
#include "util.hpp"
#include "Pub_Sub.h"

class LightController : public VrEventSubscriber {
public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	struct Light {
		Light(sf::Vector3f position, sf::Vector3f direction_cartesian, sf::Vector4f rgbi) :
			position(position), direction_cartesian(direction_cartesian), rgbi(rgbi) { 
		}
		Light();
		sf::Vector3f position;
		sf::Vector3f direction_cartesian;
		sf::Vector4f rgbi;
	};

	LightController();
	LightController(sf::Vector3f position, sf::Vector3f direction, sf::Vector4f rgbi);
	~LightController();

	void set_position(sf::Vector3f position);

	int add_static_impulse(sf::Vector3f impulse);
	int add_relative_impulse(DIRECTION direction, float speed);

	int update(double delta_time);

	void look_at_center();

	void recieve_event(VrEventPublisher* p, std::unique_ptr<vr::Event> event) override;

	static void erase_light();
	std::vector<LightController::Light>* get_lights();
private:

	// Need to allow N byte light class to be packed into 10 byte packets
	int packed_size = sizeof(Light);
	
	// Index that this light is in the packed data
	int packed_index;

	std::vector<LightController::Light> packed_data;

};


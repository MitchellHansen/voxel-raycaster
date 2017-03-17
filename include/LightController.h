#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp> 
#include "util.hpp"
#include "Pub_Sub.h"
#include "raycaster/RayCaster.h"
#include <list>
#include "raycaster/Hardware_Caster.h"

struct LightPrototype {
	
	LightPrototype(
		sf::Vector3f position,
		sf::Vector3f direction_cartesian,
		sf::Vector4f rgbi
		) : 
		position(position), 
		direction_cartesian(direction_cartesian),
		rgbi(rgbi) {	};


	sf::Vector3f position;
	sf::Vector3f direction_cartesian;
	sf::Vector4f rgbi;

	sf::Vector3f movement;
	float impulse = 1.0f;
	float friction = 1.0f;

};

// Packed data structure for passing raw light data to the caster
struct PackedData {
	PackedData(sf::Vector3f position, sf::Vector3f direction_cartesian, sf::Vector4f rgbi) :
		position(position), direction_cartesian(direction_cartesian), rgbi(rgbi) {
	}
	PackedData() {};
	~PackedData() {};
	sf::Vector4f rgbi;
	sf::Vector3f position;
	sf::Vector3f direction_cartesian;
	
};

class LightHandle;
class Hardware_Caster;

class LightController : public VrEventSubscriber {
public:

	LightController(std::shared_ptr<Hardware_Caster> raycaster);
	~LightController();

	std::shared_ptr<LightHandle> create_light(LightPrototype light_prototype);
	void remove_light(unsigned int light_index);

	void recieve_event(VrEventPublisher* p, std::unique_ptr<vr::Event> event) override;

private:

	// Set the static arrays size
	int reserved_count = 8;

	// Indices available in the light array
	std::list<unsigned int> open_list;
	
	std::vector<PackedData> packed_data_array;

};


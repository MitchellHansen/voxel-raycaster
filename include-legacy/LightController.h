#pragma once
#include <SFML/System/Vector3.hpp>
#include <list>
#include <numeric>
#include "CLCaster.h"
#include "LightHandle.h"
#include "Pub_Sub.h"
#include "util.hpp"

/**
* Light Handle :
*	- Contains data relating to movement, and a reference to the rbgi, direction, and position
*		elements in the LightController.
*	- Resultant of the use of LightController.create_light(LightPrototype). Cannot be self instantiated.
*	- On deconstruction, light data is removed from the LightController via a reference and the light disappears 
*
* LightPrototype :
*	- Contains the desired starting values for the light. The LightHandler object will then be
*		instantiated using this data
*
* PackedData :
*	- We need to single out the data that the GPU needs into a single contiguous
*		array. PackedData holds the values for position, direction, and rgbi
*
* LightController :
*	- Contains the PackedData array in a static sized array.
*		Empty light slots are set to 0 and still sent over the line
*		TODO: This introduces light limits and inefficiencies
*	- Contains a factory that takes LightPrototypes and generates unique ptr LightHandles.
*		Each light handle is given a light index enabling light removal.
*
* Typical light workflow:
* 1.) Create light prototype with desired values
* 2.) Submit prototype to the LightController 
* 3.) Get a light handle back from the controller
*		- The handle is unsafe and will break if it exceeds PackedData's lifetime
* 
*/

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
class CLCaster;

class LightController : public VrEventSubscriber {
public:

	LightController(std::shared_ptr<CLCaster> raycaster);
	~LightController();

	// find a free light 'slot' and create the light
	// LightHandles are single instance single lifetime data structures
	std::shared_ptr<LightHandle> create_light(LightPrototype light_prototype);
	
	void remove_light(unsigned int light_index);

	void event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event> event) override;

private:

	// Set the static arrays size
	int reserved_count = 8;

	// Indices available in the light array
	std::list<unsigned int> open_list;
	
	std::vector<PackedData> packed_data_array;

};


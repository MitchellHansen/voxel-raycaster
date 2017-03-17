#pragma once
#include <SFML/Graphics.hpp>
#include <util.cpp>
#include <memory>
#include "Pub_Sub.h"


// Light Handle :
//	- Contains data relating to movement, and a reference to the rbgi, direction, and position
//		elements in the LightController.
//	- Resultant of the use of LightController.create_light(LightPrototype). Cannot be self instantiated.
//	- On deconstruction, light data is removed from the LightController and the light disappears 

// LightPrototype :
//	- Contains the desired starting values for the light. The LightHandler object will then be
//		instantiated using this data

// PackedData :
//	- We need to single out the data that the GPU needs into a single contiguous
//		array. PackedData holds the values for position, direction, and rgbi

// LightController :
//	- Contains the PackedData array in a static sized array.
//		Empty light slots are set to 0 and still sent over the line
//		TODO: This introduces light limits and inefficiencies
//	- Contains a factory that takes LightPrototypes and generates unique ptr LightHandles.
//		Each light handle is given a light index enabling light removal.



struct LightPrototype;
class LightController;
struct PackedData;

class LightHandle : public VrEventSubscriber{

public:

	// Allow LightController to access this objects constructor for a factory style pattern
	friend class LightController;

	~LightHandle();

	// Functions for movement
	void set_friction(float friction);
	void set_impulse(float impulse);
	void set_movement(sf::Vector3f movement);
	void add_movement(sf::Vector3f movement);

	// Functions modifying the pointed to data
	void set_position(sf::Vector3f position);
	void set_direction(sf::Vector3f direction);
	void set_rgbi(sf::Vector4f rgbi);


	virtual void recieve_event(VrEventPublisher* publisher, std::unique_ptr<vr::Event> event) override;

	void update(double delta_time);

private:

	LightHandle(LightController *const light_controller, unsigned int light_id, LightPrototype light_prototype, PackedData *const data_reference);

	// Reference to the LightController to handle deconstruction and removal using the light_id
	LightController *const light_controller_ref;
	const unsigned int light_id;

	// Movement values provided by the prototype
	float friction_coefficient = 0.1f;
	float default_impulse = 1.0f;
	sf::Vector3f movement;

	// Reference to the packed data in the LightController
	PackedData *const data_reference;
};

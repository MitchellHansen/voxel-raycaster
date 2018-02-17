#pragma once
#include <SFML/Graphics.hpp>
#include <util.hpp>
#include <memory>
#include "Pub_Sub.h"
#include "Vector4.hpp"
#include "Gui.h"


struct LightPrototype;
class LightController;
struct PackedData;

class LightHandle : public VrEventSubscriber, private Gui{

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
	sf::Vector3f get_position();
	void set_direction(sf::Vector3f direction);
	void set_rgbi(sf::Vector4f rgbi);


	virtual void event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event> event) override;

	void update(double delta_time);


	virtual void render_gui() override;
	virtual void update_gui() override;

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

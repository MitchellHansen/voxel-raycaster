#pragma once
#include <SFML/Graphics.hpp>
#include <util.hpp>
#include <memory>


// We need to be able to :
//		- Allow lights to exist outside the context of a LightController
//			- i.e we can have a prototype light or similar
//		- Preserve the existence of lights upon object destruction
//			- No need to keep track of an object only used upon special use cases
//		- Maintain an X*N byte array, X = size of packed data, N = Light number
//			- But still allow classes of size Y bytes
//			- Preserve X packed bytes in an array which are pointed to by an array of shared pointers

struct LightPrototype;
class LightController;
struct PackedData;

class LightHandle {

public:

	friend class LightController;

	~LightHandle();

	void set_friction(float friction);
	void set_impulse(float impulse);
	void set_movement(sf::Vector3f movement);
	void add_movement(sf::Vector3f movement);

	void set_position(sf::Vector3f position);
	void set_direction(sf::Vector3f direction);
	void set_rgbi(sf::Vector4f rgbi);

private:

	LightHandle(LightController *const light_controller, unsigned int light_id, LightPrototype light_prototype, std::unique_ptr<PackedData> data_reference);

	LightController *const light_controller_ref;
	unsigned int light_id;

	float friction_coefficient = 0.1f;
	float default_impulse = 1.0f;
	sf::Vector3f movement;

	std::unique_ptr<PackedData> data_reference;
};

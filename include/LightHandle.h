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


class LightHandle {

public:
	LightHandle();
	//LightHandle(LightController light_controller, std::string light_name);
	~LightHandle();

private:

	float friction_coefficient = 0.1f;
	float default_impulse = 1.0f;
	sf::Vector3f movement;

	std::shared_ptr<sf::Vector3f> position;
	std::shared_ptr<sf::Vector3f> direction_cartesian;
	std::shared_ptr<sf::Vector4f> rgbi;
};

#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp> 
#include "util.hpp"
#include "Pub_Sub.h"

class Light : public VrEventSubscriber {
public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	Light();
	Light(sf::Vector3f position, sf::Vector3f direction);
	~Light();

	int set_position(sf::Vector3f position);

	int add_static_impulse(sf::Vector3f impulse);
	int add_relative_impulse(DIRECTION direction, float speed);

	int slew_camera(sf::Vector3f input);
	void set_camera(sf::Vector3f input);

	int update(double delta_time);

	void look_at_center();

	sf::Vector3f* get_direction_pointer();
	sf::Vector3f* get_position_pointer();
	sf::Vector3f* get_movement_pointer();

	sf::Vector3f get_movement();
	sf::Vector3f get_position();
	sf::Vector3f get_direction();


	void recieve_event(VrEventPublisher* p, std::unique_ptr<vr::Event> event) override;

private:

	// XYZ
	sf::Vector3f position;

	sf::Vector3f direction_cartesian;

	sf::Vector4f rgbi;


};


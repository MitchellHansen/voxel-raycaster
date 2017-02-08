#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp> 
#include "util.hpp"
#include "Pub_Sub.h"
#include "raycaster/RayCaster.h"
#include "LightHandle.h"


class LightController : public VrEventSubscriber {
public:

	enum DIRECTION { FORWARD, REARWARD, LEFT, RIGHT, UP, DOWN };

	// Packed data structure for passing raw light data to the caster
	struct PackedData {
		PackedData(sf::Vector3f position, sf::Vector3f direction_cartesian, sf::Vector4f rgbi) :
			position(position), direction_cartesian(direction_cartesian), rgbi(rgbi) {
		}
		PackedData() {};
		sf::Vector3f position;
		sf::Vector3f direction_cartesian;
		sf::Vector4f rgbi;
	};

//	LightController(std::shared_ptr<RayCaster> raycaster);
	LightController();
	~LightController();

	//void create_light(LightController::PackedData light_data, std::string light_name);
//	LightHandle get_light_handle(std::string light_name);

	void set_position(sf::Vector3f position);


	int update(double delta_time);

	void look_at_center();

	void recieve_event(VrEventPublisher* p, std::unique_ptr<vr::Event> event) override;

	void erase_light();
	//std::vector<LightController::PackedData>* get_lights();
private:


	//// Need to allow N byte light class to be packed into 10 byte packets
	//int packed_size = sizeof(PackedData);

	std::vector<PackedData> packed_data_array;
	//
	//
	std::unordered_map<std::string, LightHandle> light_map;
	//
	//std::shared_ptr<RayCaster> raycaster;


};


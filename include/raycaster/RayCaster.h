#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <Map.h>
#include "Old_Map.h"
#include "Camera.h"

struct PackedData;

class RayCaster {
public:

	enum ERROR_CODES {
		SHARING_NOT_SUPPORTED = 800,
		OPENCL_NOT_SUPPORTED = 801,
		OPENCL_ERROR = 802,
		ERR = 803
	};

	RayCaster();
	virtual ~RayCaster();

	virtual int init() = 0;

	virtual void assign_map(Old_Map *map) = 0;
	virtual void assign_camera(Camera *camera) = 0;
	virtual void create_viewport(int width, int height, float v_fov, float h_fov) = 0;
	virtual void assign_lights(std::vector<PackedData> *data) = 0;
	virtual void validate() = 0;

	// draw will abstract the gl sharing and software rendering
	// methods of retrieving the screen buffer
	virtual void compute() = 0;
	virtual void draw(sf::RenderWindow* window) = 0;

protected:

	sf::Sprite viewport_sprite;
	sf::Texture viewport_texture;

	Old_Map * map = nullptr;
	Camera *camera = nullptr;
//	std::vector<LightController::PackedData> *lights;
	std::vector<PackedData> *lights;
	int light_count = 0;
	sf::Uint8 *viewport_image = nullptr;
	sf::Vector4f *viewport_matrix = nullptr;
	sf::Vector2i viewport_resolution;

	int error = 0;

};


#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <Map.h>
#include "Old_map.h"
#include "Camera.h"


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
	virtual void assign_light(std::string light_id, Light light) = 0;

	// draw will abstract the gl sharing and software rendering
	// methods of retrieving the screen buffer
	virtual void draw(sf::RenderWindow* window) = 0;

protected:

	sf::Sprite viewport_sprite;
	sf::Texture viewport_texture;

	Old_Map * map;
	Camera *camera;
	std::map<std::string, Light> light_map;
	sf::Uint8 *viewport_image;
	sf::Vector4f *viewport_matrix;

	int error = 0;

};


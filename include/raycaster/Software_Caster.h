#pragma once
#include "raycaster/RayCaster.h"
#include <thread>
#include "map/Old_Map.h"
#include "Camera.h"

struct PackedData;

class Software_Caster : public RayCaster
{
public:
	Software_Caster();

	virtual ~Software_Caster();

	int init() override;

	// In interop mode, this will create a GL texture that we share
	// Otherwise, it will create the pixel buffer and pass that in as an image, retrieving it each draw
	// Both will create the view matrix, view res buffer
	void create_viewport(int width, int height, float v_fov, float h_fov) override;

	void assign_lights(std::vector<PackedData> *data) override;
	void assign_map(Old_Map *map) override;
	void assign_camera(Camera *camera) override;
	void validate() override;

	// draw will abstract the gl sharing and software rendering
	// methods of retrieving the screen buffer
	void compute() override;
	void draw(sf::RenderWindow* window) override;

private:
	
	void cast_viewport();
	void cast_thread(int start_id, int end_id);
	void cast_ray(int id);
	void blit_pixel(sf::Color color, sf::Vector2i position, sf::Vector3i mask);
	sf::Color global_light(sf::Color in, sf::Vector3i mask);
};

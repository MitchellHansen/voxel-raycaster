#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "SFML/Graphics.hpp"

#include "Camera.h"
#include "Old_Map.h"
#include "RayCaster.h"

// Renderer needs to handle the distinction between a few difference circumstances.
// A.) The machine supports OpenCL and cl_khr_gl_sharing
//          Everything is normal, rendering is handled on-gpu
// B.) The machine support Opencl and NOT cl_khr_gl_sharing
//          For every frame we have to pull the screen buffer from the GPU's memory
// C.) The machine does not support OpenCL
//          We must use the fallback software renderer

// Renderer will hold its own CL_Renderer class which contains all of the data
// and functionality that the CL_Wrapper class currently does, but with the
// intent of leaving it specialized to only the raycaster. Any further OpenCL
// work can use its own class

// Perhaps in the future there will be a container "scene" which will
// hold the current map, camera, and light objects. The renderer will
// then be passed that scene which it will then use to render with

class Renderer {

public:
    Renderer();

    // The renderer needs all of the things that are required
    // by CL in order to render the screen
    void register_camera(Camera* camera);
	void register_map(Old_Map* map);
	void register_lights();
	void create_viewport(float v_fov, float h_fov, int height, int width);
	void register_light(LightController l);

    void draw();
    sf::RenderWindow* get_window();

private:

	RayCaster *rc;

    bool sharing_supported = false;
	bool cl_supported = false;
    
	sf::Uint8 *drawing_surface;
    sf::RenderWindow* window;

	std::vector<LightController> lights;
	Old_Map* map;
	Camera* camera;
	sf::Uint8 *view_matrix;

};


#endif

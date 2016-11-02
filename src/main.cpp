#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
#include <windows.h>
#include <CL/cl_gl.h>
#include <CL/cl.h>
#include <CL/opencl.h>
#include <GL/GL.h>

#include <windows.h>

#elif defined TARGET_OS_MAC
#include <OpenGL/gl.h>
# include <OpenGL/OpenGL.h>
# include <OpenCL/opencl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_ext.h>

#endif

#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

#include "Old_Map.h"
#include "Curses.h"
#include "util.hpp"
#include "RayCaster.h"
#include "Hardware_Caster.h"
#include "CL_Wrapper.h"
#include "Vector4.hpp"
#include <Camera.h>

const int WINDOW_X = 1920;
const int WINDOW_Y = 1080;
const int WORK_SIZE = WINDOW_X * WINDOW_Y;

const int MAP_X = 512;
const int MAP_Y = 512;
const int MAP_Z = 512;

float elap_time(){
	static std::chrono::time_point<std::chrono::system_clock> start;
	static bool started = false;

	if (!started){
		start = std::chrono::system_clock::now();
		started = true;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = now - start;
	return static_cast<float>(elapsed_time.count());
}

sf::Sprite window_sprite;
sf::Texture window_texture;

// Y: -1.57 is straight up
// Y: 1.57 is straight down


int main() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");

	RayCaster rc = new Hardware_Caster()
	CL_Wrapper c;

	if (c.compile_kernel("../kernels/ray_caster_kernel.cl", true, "min_kern") < 0) {
		std::cin.get();
		return -1;
	}
	
	std::cout << "map...";
	sf::Vector3i map_dim(MAP_X, MAP_Y, MAP_Z);
    Old_Map* map = new Old_Map(map_dim);
	map->generate_terrain();

	c.create_buffer("map_buffer", sizeof(char) * map_dim.x * map_dim.y * map_dim.z, map->get_voxel_data());
	c.create_buffer("dim_buffer", sizeof(int) * 3, &map_dim);

	sf::Vector2i view_res(WINDOW_X, WINDOW_Y);
	c.create_buffer("res_buffer", sizeof(int) * 2, &view_res);
  	

    double y_increment_radians = DegreesToRadians(50.0f / view_res.y);
    double x_increment_radians = DegreesToRadians(80.0f / view_res.x);

	std::cout << "view matrix...";
  
	sf::Vector4f* view_matrix = new sf::Vector4f[WINDOW_X * WINDOW_Y * 4];

    for (int y = -view_res.y / 2; y < view_res.y / 2; y++) {
        for (int x = -view_res.x / 2; x < view_res.x / 2; x++) {

            // The base ray direction to slew from
            sf::Vector3f ray(1, 0, 0);

			// Y axis, pitch
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(y_increment_radians * y) + ray.x * cos(y_increment_radians * y)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(y_increment_radians * y) - ray.x * sin(y_increment_radians * y))
			);


			// Z axis, yaw
			ray = sf::Vector3f(
				static_cast<float>(ray.x * cos(x_increment_radians * x) - ray.y * sin(x_increment_radians * x)),
				static_cast<float>(ray.x * sin(x_increment_radians * x) + ray.y * cos(x_increment_radians * x)),
				static_cast<float>(ray.z)
			);
            
			int index = (x + view_res.x / 2) + view_res.x * (y + view_res.y / 2);
            ray = Normalize(ray);

            view_matrix[index] = sf::Vector4f(
				ray.x,
				ray.y,
				ray.z,
				0
			);
        }
    }

	c.create_buffer("view_matrix_buffer", sizeof(float) * 4 * view_res.x * view_res.y, view_matrix);

	Camera camera(
		sf::Vector3f(0, 0, 0),
		sf::Vector2f(0.0f, 1.00f)
	);
	
	c.create_buffer("cam_dir_buffer", sizeof(float) * 4, (void*)camera.get_direction_pointer(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	c.create_buffer("cam_pos_buffer", sizeof(float) * 4, (void*)camera.get_position_pointer(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
    
	int light_count = 2;
	c.create_buffer("light_count_buffer", sizeof(int), &light_count);

	// {r, g, b, i, x, y, z, x', y', z'}
	sf::Vector3f v = Normalize(sf::Vector3f(1.0f, 0.0f, 0.0f));
	sf::Vector3f v2 = Normalize(sf::Vector3f(1.1f, 0.4f, 0.7f));
	float light[] = { 0.4f, 0.8f, 0.1f, 1.0f, 50.0f, 50.0f, 50.0f, v.x, v.y, v.z,
					  0.4f, 0.8f, 0.1f, 1.0f, 50.0f, 50.0f, 50.0f, v2.x, v2.y, v2.z};
	c.create_buffer("light_buffer", sizeof(float) * 10 * light_count, light, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

	// The drawing canvas
    unsigned char* pixel_array = new sf::Uint8[WINDOW_X * WINDOW_Y * 4];

    for (int i = 0; i < WINDOW_X * WINDOW_Y * 4; i += 4) {

        pixel_array[i] = 255; // R?
        pixel_array[i + 1] = 255; // G?
        pixel_array[i + 2] = 255; // B?
        pixel_array[i + 3] = 100; // A?
    }

	sf::Texture t;
    t.create(WINDOW_X, WINDOW_Y);
    t.update(pixel_array);

    int error;
    cl_mem image_buff = clCreateFromGLTexture(
            c.getContext(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
            0, t.getNativeHandle(), &error);

    if (c.assert(error, "clCreateFromGLTexture"))
        return -1;

    c.store_buffer(image_buff, "image_buffer");

    c.set_kernel_arg("min_kern", 0, "map_buffer");
    c.set_kernel_arg("min_kern", 1, "dim_buffer");
    c.set_kernel_arg("min_kern", 2, "res_buffer");
    c.set_kernel_arg("min_kern", 3, "view_matrix_buffer");
    c.set_kernel_arg("min_kern", 4, "cam_dir_buffer");
    c.set_kernel_arg("min_kern", 5, "cam_pos_buffer");
	c.set_kernel_arg("min_kern", 6, "light_buffer");
	c.set_kernel_arg("min_kern", 7, "light_count_buffer");
	c.set_kernel_arg("min_kern", 8, "image_buffer");

	sf::Sprite s;
	s.setTexture(t);
	s.setPosition(0, 0);

    // The step size in milliseconds between calls to Update()
    // Lets set it to 16.6 milliseonds (60FPS)
    float step_size = 0.0166f;

    // Timekeeping values for the loop
    double  frame_time = 0.0,
            elapsed_time = 0.0,
            delta_time = 0.0,
            accumulator_time = 0.0,
            current_time = 0.0;

    fps_counter fps;

	// ============================= RAYCASTER SETUP ==================================

	// Setup the sprite and texture
	window_texture.create(WINDOW_X, WINDOW_Y);
	window_sprite.setPosition(0, 0);

	// State values

	sf::Vector3f cam_vec(0, 0, 0);

	//RayCaster ray_caster(map, map_dim, view_res);

	sf::Vector2f *dp = camera.get_direction_pointer();
	debug_text cam_text_x(1, 30, &dp->x, "incli: ");
	debug_text cam_text_y(2, 30, &dp->y, "asmth: ");

	sf::Vector3f *mp = camera.get_movement_pointer();
	debug_text cam_text_mov_x(4, 30, &mp->x, "X: ");
	debug_text cam_text_mov_y(5, 30, &mp->y, "Y: ");
	debug_text cam_text_mov_z(6, 30, &mp->y, "Z: ");
	//debug_text cam_text_z(3, 30, &p->z);

	debug_text light_x(7, 30, &light[7], "X: ");
	debug_text light_y(8, 30, &light[8], "Y: ");
	debug_text light_z(9, 30, &light[9], "Z: ");
	// ===============================================================================

	// Mouse capture
	sf::Vector2i deltas;
	sf::Vector2i fixed(window.getSize());
	bool mouse_enabled = true;

	sf::Vector3f cam_mov_vec;

	while (window.isOpen()) {

		// Poll for events from the user
		sf::Event event;
		while (window.pollEvent(event)) {

			// If the user tries to exit the application via the GUI
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Space) {
					if (mouse_enabled)
						mouse_enabled = false;
					else
						mouse_enabled = true;
				}
			}
		}

		cam_vec.x = 0;
		cam_vec.y = 0;
		cam_vec.z = 0;

		float speed = 1.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
			speed = 0.2f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			camera.add_relative_impulse(Camera::DIRECTION::DOWN, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			camera.add_relative_impulse(Camera::DIRECTION::UP, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			camera.add_relative_impulse(Camera::DIRECTION::FORWARD, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			camera.add_relative_impulse(Camera::DIRECTION::REARWARD, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			camera.add_relative_impulse(Camera::DIRECTION::LEFT, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			camera.add_relative_impulse(Camera::DIRECTION::RIGHT, speed);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
			camera.set_position(sf::Vector3f(50, 50, 50));
		}

		camera.add_static_impulse(cam_vec);

		if (mouse_enabled) {
			deltas = fixed - sf::Mouse::getPosition();
			if (deltas != sf::Vector2i(0, 0) && mouse_enabled == true) {

				// Mouse movement
				sf::Mouse::setPosition(fixed);
				camera.slew_camera(sf::Vector2f(
					deltas.y / 300.0f,
					deltas.x / 300.0f
				));
			}
		}

        // Time keeping
		elapsed_time = elap_time();
		delta_time = elapsed_time - current_time;
		current_time = elapsed_time;
		if (delta_time > 0.2f)
			delta_time = 0.2f;
		accumulator_time += delta_time;
		while ((accumulator_time - step_size) >= step_size) {
            accumulator_time -= step_size;

            // ==== DELTA TIME LOCKED ====
        }

		float l[] = {
			static_cast<float>(light[9] * sin(delta_time / 1) + light[7] * cos(delta_time / 1)),
			static_cast<float>(light[8]),
			static_cast<float>(light[9] * cos(delta_time / 1) - light[7] * sin(delta_time / 1))
		};

		float l2[] = {
			static_cast<float>(l[0] * cos(delta_time) - l[2] * sin(delta_time)),
			static_cast<float>(l[0] * sin(delta_time) + l[2] * cos(delta_time)),
			static_cast<float>(l[2])
		};

		light[7] = l[0];
		light[8] = l[1];
		light[9] = l[2];

        // ==== FPS LOCKED ====
		camera.update(delta_time);

		// Run the raycast
		c.run_kernel("min_kern", WORK_SIZE);
				
		// ==== RENDER ====

		window.clear(sf::Color::Black);

        window.draw(s);

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(delta_time);
		fps.draw(&window);

		cam_text_x.draw(&window);
		cam_text_y.draw(&window);

		cam_text_mov_x.draw(&window);
		cam_text_mov_y.draw(&window);
		cam_text_mov_z.draw(&window);

		light_x.draw(&window);
		light_y.draw(&window);
		light_z.draw(&window);
			
		window.display();

	}
	return 0;
}

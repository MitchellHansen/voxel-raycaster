#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
#include <CL/cl_gl.h>
#include <CL/cl.h>
#include <CL/opencl.h>
#include <windows.h>

#elif defined TARGET_OS_MAC
#include <OpenGL/gl.h>
# include <OpenGL/OpenGL.h>
# include <OpenCL/opencl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_ext.h>

#endif
#include "TestPlatform.cpp"
#include "Map.h"
#include "Curses.h"
#include "util.hpp"
#include "RayCaster.h"
#include "CL_Wrapper.h"

const int WINDOW_X = 150;
const int WINDOW_Y = 150;





float elap_time(){
	static std::chrono::time_point<std::chrono::system_clock> start;
	static bool started = false;

	if (!started){
		start = std::chrono::system_clock::now();
		started = true;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = now - start;
	return elapsed_time.count();
}

sf::Sprite window_sprite;
sf::Texture window_texture;

// Y: -1.57 is straight up
// Y: 1.57 is straight down


int main() {

    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");


    sf::Sprite s;
    sf::Texture t;

    {
        CL_Wrapper c;
        c.acquire_platform_and_device();
        c.create_shared_context();
        c.create_command_queue();

        c.compile_kernel("../kernels/kernel.c", true, "hello");
        c.compile_kernel("../kernels/minimal_kernel.c", true, "min_kern");

        std::string in = "hello!!!!!!!!!!!!!!!!!!!!!";
        cl_mem buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                sizeof(char) * 128, &in[0], NULL
        );

        char map[100 * 100 * 100];

        for (int i = 0; i < 100 * 100 * 100; i++) {
            map[i] = '+';
        }

        map[0] = 'a';

        cl_mem map_buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(char) * 100 * 100 * 100, map, NULL
        );

        int dim[3] = {101, 100, 99};

        cl_mem dim_buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(int) * 3, dim, NULL
        );

        int res[2] = {100, 99};

        cl_mem res_buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(int) * 2, res, NULL
        );

        double y_increment_radians = DegreesToRadians(50.0 / res[1]);
        double x_increment_radians = DegreesToRadians(80.0 / res[0]);

        // SFML 2.4 has Vector4 datatypes.......

        float view_matrix[res[0] * res[1] * 4];
        for (int y = -res[1] / 2; y < res[1] / 2; y++) {
            for (int x = -res[0] / 2; x < res[0] / 2; x++) {

                // The base ray direction to slew from
                sf::Vector3f ray(1, 0, 0);

                // Y axis, pitch
                ray = sf::Vector3f(
                        ray.z * sin(y_increment_radians * y) + ray.x * cos(y_increment_radians * y),
                        ray.y,
                        ray.z * cos(y_increment_radians * y) - ray.x * sin(y_increment_radians * y)
                );

                // Z axis, yaw
                ray = sf::Vector3f(
                        ray.x * cos(x_increment_radians * x) - ray.y * sin(x_increment_radians * x),
                        ray.x * sin(x_increment_radians * x) + ray.y * cos(x_increment_radians * x),
                        ray.z
                );

                int index = (x + res[0] / 2) + res[0] * (y + res[1] / 2);
                ray = Normalize(ray);
                view_matrix[index * 4 + 0] = ray.x;
                view_matrix[index * 4 + 1] = ray.y;
                view_matrix[index * 4 + 2] = ray.z;
                view_matrix[index * 4 + 3] = 0;
            }
        }

//    int ind = 4;
//    std::cout << "\nX: " << view_matrix[ind]
//              << "\nY: " << view_matrix[ind + 1]
//              << "\nZ: " << view_matrix[ind + 2]
//              << "\npad: " << view_matrix[ind + 3];
//
//    std::cout << "\n======================" << std::endl;

        cl_mem view_matrix_buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float) * 3 * res[0] * res[1], view_matrix, NULL
        );


        float cam_dir[4] = {1, 0, 0, 0};

        cl_mem cam_dir_buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float) * 4, cam_dir, NULL
        );

        float cam_pos[4] = {25, 25, 25, 0};

        cl_mem cam_pos_buff = clCreateBuffer(
                c.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float) * 4, cam_pos, NULL
        );


        c.store_buffer(buff, "buffer_1");
        c.store_buffer(map_buff, "map_buffer");
        c.store_buffer(dim_buff, "dim_buffer");
        c.store_buffer(res_buff, "res_buffer");
        c.store_buffer(view_matrix_buff, "view_matrix_buffer");
        c.store_buffer(cam_dir_buff, "cam_dir_buffer");
        c.store_buffer(cam_pos_buff, "cam_pos_buffer");

        c.set_kernel_arg("min_kern", 0, "buffer_1");
        c.set_kernel_arg("min_kern", 1, "map_buffer");
        c.set_kernel_arg("min_kern", 2, "dim_buffer");
        c.set_kernel_arg("min_kern", 3, "res_buffer");
        c.set_kernel_arg("min_kern", 4, "view_matrix_buffer");
        c.set_kernel_arg("min_kern", 5, "cam_dir_buffer");
        c.set_kernel_arg("min_kern", 6, "cam_pos_buffer");

        c.run_kernel("min_kern");


        unsigned char* pixel_array = new sf::Uint8[WINDOW_X * WINDOW_Y * 4];

        for (int i = 0; i < 100 * 100 * 4; i += 4) {

            pixel_array[i] = i % 255; // R?
            pixel_array[i + 1] = 70; // G?
            pixel_array[i + 2] = 100; // B?
            pixel_array[i + 3] = 100; // A?
        }

        t.create(100, 100);
        t.update(pixel_array);

        int error;

        cl_mem image_buff = clCreateFromGLTexture(c.getContext(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, t.getNativeHandle(), &error);
        if (c.assert(error, "clCreateFromGLTexture"))
            return -1;

        error = clEnqueueAcquireGLObjects(c.getCommandQueue(), 1, &image_buff, 0, 0, 0);
        if (c.assert(error, "clEnqueueAcquireGLObjects"))
            return -1;

        //c.run_kernel("min_kern");

        error = clEnqueueReleaseGLObjects(c.getCommandQueue(), 1, &image_buff, 0, NULL, NULL);
        if (c.assert(error, "clEnqueueReleaseGLObjects"))
            return -1;

        s.setTexture(t);


    }
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
	sf::Vector3i map_dim(100, 100, 100);
	sf::Vector2i view_res(WINDOW_X, WINDOW_Y);
	sf::Vector3f cam_dir(1.0f, 0.0f, 1.57f);
	sf::Vector3f cam_pos(50, 50, 50);
	sf::Vector3f cam_vec(0, 0, 0);
	Map* map = new Map(map_dim);
	RayCaster ray_caster(map, map_dim, view_res);


	// ===============================================================================

	// Mouse capture
	sf::Vector2i deltas;
	sf::Vector2i fixed(window.getSize());
	bool mouse_enabled = true;

	while (window.isOpen()) {

		// Poll for events from the user
		sf::Event event;
		while (window.pollEvent(event)) {

			// If the user tries to exit the application via the GUI
			if (event.type == sf::Event::Closed)
				window.close();
		}

		cam_vec.x = 0;
		cam_vec.y = 0;
		cam_vec.z = 0;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			cam_vec.z = 1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			cam_vec.z = -1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			cam_vec.y = 1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			cam_vec.y = -1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			cam_vec.x = 1;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			cam_vec.x = -1;
		}
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            cam_dir.z = -0.1f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            cam_vec.z = +0.1f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            cam_vec.y = +0.1f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            cam_vec.y = -0.1f;
        }

		deltas = fixed - sf::Mouse::getPosition();
		if (deltas != sf::Vector2i(0, 0) && mouse_enabled == true) {

			// Mouse movement
			sf::Mouse::setPosition(fixed);
			cam_dir.y -= deltas.y / 300.0f;
			cam_dir.z -= deltas.x / 300.0f;
		}

		cam_pos.x += cam_vec.x / 1.0;
		cam_pos.y += cam_vec.y / 1.0;
		cam_pos.z += cam_vec.z / 1.0;

		std::cout << cam_vec.x << " : " << cam_vec.y << " : " << cam_vec.z << std::endl;


        // Time keeping
		elapsed_time = elap_time();
		delta_time = elapsed_time - current_time;
		current_time = elapsed_time;
		if (delta_time > 0.2f)
			delta_time = 0.2f;
		accumulator_time += delta_time;
		while ((accumulator_time - step_size) >= step_size) {
            accumulator_time -= step_size;

            // Update cycle
        }

        // Fps cycle
        // map->moveLight(sf::Vector2f(0.3, 0));

		window.clear(sf::Color::Black);

		// Cast the rays and get the image
		sf::Color* pixel_colors = ray_caster.CastRays(cam_dir, cam_pos);

        // Cast it to an array of Uint8's
		auto out = (sf::Uint8*)pixel_colors;

        window_texture.update(out);
		window_sprite.setTexture(window_texture);
		window.draw(window_sprite);

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(delta_time);
		fps.draw(&window);


        window.draw(s);
        
		window.display();

	}
	return 0;
}

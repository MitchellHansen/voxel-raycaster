#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include "util.hpp"
#include "RayCaster.h"
#include <Map.h>

const int WINDOW_X = 200;
const int WINDOW_Y = 200;


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

void test_ray_reflection(){

    sf::Vector3f r(0.588, -0.78, -0.196);
    sf::Vector3f i(0, 0.928, 0.37);

    // is this needed? free spin but bounded 0 < z < pi
    if (i.z > PI)
        i.z -= PI;
    else if (i.z < 0)
        i.z += PI;

    std::cout << AngleBetweenVectors(r, i);



    return;
}

int main() {

    // Initialize the render window
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");

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
	sf::Vector3i map_dim(50, 50, 50);
	sf::Vector2i view_res(WINDOW_X, WINDOW_Y);
	sf::Vector3f cam_dir(1.0f, 0.0f, 1.57f);
	sf::Vector3f cam_pos(50, 50, 50);

	Map* map = new Map(map_dim);
	RayCaster ray_caster(map, map_dim, view_res);


	// ===============================================================================


	while (window.isOpen()) {

		// Poll for events from the user
		sf::Event event;
		while (window.pollEvent(event)) {

			// If the user tries to exit the application via the GUI
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {

				// CAMERA DIRECTION
				if (event.key.code == sf::Keyboard::Left) {
					cam_dir.z -= 0.1f;
					std::cout << "X:" << cam_dir.x << " Y:" << cam_dir.y << " Z:" << cam_dir.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::Right) {
					cam_dir.z += 0.1f;
					std::cout << "X:" << cam_dir.x << " Y:" << cam_dir.y << " Z:" << cam_dir.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::Down) {
					cam_dir.y += 0.1f;
					std::cout << "X:" << cam_dir.x << " Y:" << cam_dir.y << " Z:" << cam_dir.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::Up) {
					cam_dir.y -= 0.1f;
					std::cout << "X:" << cam_dir.x << " Y:" << cam_dir.y << " Z:" << cam_dir.z << std::endl;
				}

				// CAMERA POSITION
				if (event.key.code == sf::Keyboard::Q) {
					cam_pos.z -= 1;
					std::cout << "X:" << cam_pos.x << " Y:" << cam_pos.y << " Z:" << cam_pos.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::E) {
					cam_pos.z += 1;
					std::cout << "X:" << cam_pos.x << " Y:" << cam_pos.y << " Z:" << cam_pos.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::W) {
					cam_pos.y += 1;
					std::cout << "X:" << cam_pos.x << " Y:" << cam_pos.y << " Z:" << cam_pos.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::S) {
					cam_pos.y -= 1;
					std::cout << "X:" << cam_pos.x << " Y:" << cam_pos.y << " Z:" << cam_pos.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::A) {
					cam_pos.x += 1;
					std::cout << "X:" << cam_pos.x << " Y:" << cam_pos.y << " Z:" << cam_pos.z << std::endl;
				}
				if (event.key.code == sf::Keyboard::D) {
					cam_pos.x -= 1;
					std::cout << "X:" << cam_pos.x << " Y:" << cam_pos.y << " Z:" << cam_pos.z << std::endl;
				}
			}
		}

		// Get the elapsed time from the start of the application
		elapsed_time = elap_time();

		// Find the time that passed between now and the last frame
		delta_time = elapsed_time - current_time;

		// Set the time for the next frame to use
		current_time = elapsed_time;

		// If the time between the last frame and now was too large (lag)
		// cull the time to a more acceptable value. So instead of jumping large
		// amounts when lagging, the app only jumps in set increments
		if (delta_time > 0.2f)
			delta_time = 0.2f;

		// Add the frame time to the accumulator, a running total of time we
		// need to account for in the application
		accumulator_time += delta_time;

		// While we have time to step
		while ((accumulator_time - step_size) >= step_size) {

			// Take away a step from the accumulator
			accumulator_time -= step_size;


			// And update the application for the amount of time alotted for one step
			// Update(step_size);
		}

        map->moveLight(sf::Vector2f(0.3, 0));
		
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

		window.display();



	}
	return 0;

}

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include "util.hpp"
#include "../build/RayCaster.h"

const int WINDOW_X = 600;
const int WINDOW_Y = 800;


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
	sf::Vector3i map_dim(100, 100, 100);
	sf::Vector2i view_res(200, 200);
	sf::Vector3f cam_dir(1.0f, 0.0f, 1.57f);
	sf::Vector3f cam_pos(10, 10, 10);

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
		if (delta_time > 0.05f)
			delta_time = 0.05f;

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

		
		window.clear(sf::Color::Black);

		// Cast the rays and get the image
		sf::Color* pixel_colors = ray_caster.CastRays(cam_dir, cam_pos);

		/*for (int i = 0; i < img_size; i++) {
			pixel_colors[i] = sf::Color::Green;
		}*/

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

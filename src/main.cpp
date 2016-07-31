#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include "util.hpp"

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


		// Rendering code goes here
		window.clear(sf::Color::Black);

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(delta_time);
		fps.draw(&window);

		window.display();



	}
	return 0;

}

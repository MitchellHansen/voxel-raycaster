#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <random>

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
	elap_time();
	std::mt19937 rng(time(NULL));
	std::uniform_int_distribution<int> rgen(100, 400);

	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");

	float step_size = 0.005f;
	double frame_time = 0.0, elapsed_time = 0.0, delta_time = 0.0, accumulator_time = 0.0, current_time = 0.0;


	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		elapsed_time = elap_time();
		delta_time = elapsed_time - current_time;
		current_time = elapsed_time;
		if (delta_time > 0.02f)
			delta_time = 0.02f;
		accumulator_time += delta_time;

		while ((accumulator_time - step_size) >= step_size) {
			accumulator_time -= step_size;

			// Update(step_size);
		}

		window.clear(sf::Color::Black);

		window.display();



	}
	return 0;

}

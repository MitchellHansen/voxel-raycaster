#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

const double PI = 3.141592653589793238463;
const float  PI_F = 3.14159265358979f;

struct fps_counter {
public:
		fps_counter(){
				if(!f.loadFromFile("../assets/fonts/Arial.ttf")){
						std::cout << "couldn't find the fall back Arial font in ../assets/fonts/" << std::endl;
				} else {
						t.setFont(f);
				}
		}

		void frame(double delta_time){
				frame_count++;
				fps_average += (delta_time - fps_average) / frame_count;
		}
		
		void draw(sf::RenderWindow *r){
				t.setString(std::to_string(fps_average));
				r->draw(t);
		}

private:
		sf::Font f;
		sf::Text t;
		int frame_count = 0;
		double fps_average = 0;
};


inline sf::Vector3f SphereToCart(sf::Vector3f i) {

	auto r = sf::Vector3f(
		(i.x * sin(i.z) * cos(i.y)),
		(i.x * sin(i.z) * sin(i.y)),
		(i.x * cos(i.z))
		);
	return r;
};


inline sf::Vector3f CartToSphere(sf::Vector3f in) {

	auto r = sf::Vector3f(
		sqrt(in.x * in.x + in.y * in.y + in.z * in.z),
		atan(in.y / in.x),
		atan(sqrt(in.x * in.x + in.y * in.y) / in.z)
		);
	return r;
};


inline sf::Vector3f Normalize(sf::Vector3f in) {
	
	float multiplier = sqrt(in.x * in.x + in.y * in.y + in.z * in.z);
	auto r = sf::Vector3f(
		in.x / multiplier,
		in.y / multiplier,
		in.z / multiplier
	);
	return r;

}

inline float DegreesToRadians(float in) {
	return in * PI / 180.0f;
}

inline float RadiansToDegrees(float in) {
	return in * 180.0f / PI;
}
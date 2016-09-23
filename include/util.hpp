#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>

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
			if (frame_count == 100){
				frame_count = 0;
				fps_average = 0;
			}
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

struct debug_text {
public:
	debug_text(int slot, int pixel_spacing, void* data_, std::string prefix_) : data(data_), prefix(prefix_) {
		if (!f.loadFromFile("../assets/fonts/Arial.ttf")) {
			std::cout << "couldn't find the fall back Arial font in ../assets/fonts/" << std::endl;
		}
		else {
			t.setFont(f);
			t.setCharacterSize(20);
			t.setPosition(20, slot * pixel_spacing);
		}

	}

	void draw(sf::RenderWindow *r) {
		t.setString(prefix + std::to_string(*(float*)data));
		r->draw(t);
	}

private:
	void* data;
	std::string prefix;
	sf::Font f;
	sf::Text t;

};

inline sf::Vector3f SphereToCart(sf::Vector2f i) {

	auto r = sf::Vector3f(
		(1 * sin(i.y) * cos(i.x)),
		(1 * sin(i.y) * sin(i.x)),
		(1 * cos(i.y))
		);
	return r;
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


inline float DotProduct(sf::Vector3f a, sf::Vector3f b){
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float Magnitude(sf::Vector3f in){
	return sqrt(in.x * in.x + in.y * in.y + in.z * in.z);
}

inline float AngleBetweenVectors(sf::Vector3f a, sf::Vector3f b){
	return acos(DotProduct(a, b) / (Magnitude(a) * Magnitude(b)));
}

inline float DegreesToRadians(float in) {
	return in * PI / 180.0f;
}

inline float RadiansToDegrees(float in) {
	return in * 180.0f / PI;
}

inline std::string read_file(std::string file_name){
	std::ifstream input_file(file_name);

	if (!input_file.is_open()){
		std::cout << file_name << " could not be opened" << std::endl;
		return nullptr;
	}

	std::stringstream buf;
	buf << input_file.rdbuf();
	return buf.str();
}

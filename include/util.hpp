#pragma once
#include <SFML/Graphics.hpp>
#include "Vector4.hpp"
#include <math.h>
#include <string.h>
#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <list>
#include <algorithm>
#include <imgui/imgui.h>

const double PI = 3.141592653589793238463;
const float  PI_F = 3.14159265358979f;
struct fps_counter {
public:
	fps_counter() {};

	void frame(double delta_time) {
		// Apply 100 units of smoothing
		if (frame_count == 100) {
			frame_count = 0;
			fps_average = 0;
		}
		frame_count++;
		fps_average += (delta_time - fps_average) / frame_count;
	}

	void draw() {

		if (arr_pos == 200)
			arr_pos = 0;

		fps_array[arr_pos] = static_cast<float>(1.0 / fps_average);
		arr_pos++;

		ImGui::Begin("Performance");
		ImGui::PlotLines("FPS", fps_array, 200, 0, std::to_string(1.0 / fps_average).c_str(), 0.0f, 150.0f, ImVec2(200, 80));
		ImGui::End();
	}

private:

	float fps_array[200]{60};
	int arr_pos = 0;

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
			t.setPosition(static_cast<float>(20), static_cast<float>(slot * pixel_spacing));
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

inline sf::Vector2f CartToNormalizedSphere(sf::Vector3f in) {

	auto r = sf::Vector2f(
		atan2(sqrt(in.x * in.x + in.y * in.y), in.z), 
		atan2(in.y, in.x)
		);
	
	return r;
}

inline sf::Vector3f FixOrigin(sf::Vector3f base, sf::Vector3f head) {
	return head - base;
}


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

inline float DistanceBetweenPoints(sf::Vector3f a, sf::Vector3f b) {
	return sqrt(DotProduct(a, b));
}

inline float DegreesToRadians(float in) {
	return static_cast<float>(in * PI / 180.0f);
}

inline float RadiansToDegrees(float in) {
	return static_cast<float>(in * 180.0f / PI);
}

inline std::string read_file(std::string file_name){
	std::ifstream input_file(file_name);

	if (!input_file.is_open()){
		std::cout << file_name << " could not be opened" << std::endl;
		return "";
	}

	std::stringstream buf;
	buf << input_file.rdbuf();
	input_file.close();
	return buf.str();
}

inline void PrettyPrintUINT64(uint64_t i, std::stringstream* ss) {

	*ss << "[" << std::bitset<15>(i) << "]";
	*ss << "[" << std::bitset<1>(i >> 15) << "]";
	*ss << "[" << std::bitset<8>(i >> 16) << "]";
	*ss << "[" << std::bitset<8>(i >> 24) << "]";
	*ss << "[" << std::bitset<32>(i >> 32) << "]";

}

inline void PrettyPrintUINT64(uint64_t i) {

	std::cout << "[" << std::bitset<15>(i) << "]";
	std::cout << "[" << std::bitset<1>(i >> 15) << "]";
	std::cout << "[" << std::bitset<8>(i >> 16) << "]";
	std::cout << "[" << std::bitset<8>(i >> 24) << "]";
	std::cout << "[" << std::bitset<32>(i >> 32) << "]" << std::endl;

}

inline void DumpLog(std::stringstream* ss, std::string file_name) {
	
	std::ofstream log_file;
	log_file.open(file_name);

	log_file << ss->str();

	log_file.close();

}

inline std::string sfml_get_input(sf::RenderWindow *window) {
	
	std::stringstream ss;

	sf::Event event;
	while (window->pollEvent(event)) {
		if (event.type == sf::Event::TextEntered) {
			ss << event.text.unicode;
		} 
		
		else if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Return) {
				return ss.str();
			}
		}
	}
}

inline std::vector<float> sfml_get_float_input(sf::RenderWindow *window) {

	std::stringstream ss;

	sf::Event event;
	while (true) {

		if (window->pollEvent(event)) {

			if (event.type == sf::Event::TextEntered) {
				if (event.text.unicode > 47 && event.text.unicode < 58 || event.text.unicode == 32)
					ss << static_cast<char>(event.text.unicode);
			}

			else if (event.type == sf::Event::KeyPressed) {

				if (event.key.code == sf::Keyboard::Return) {
					break;
				}
			}
		}
	}

	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> vstrings(begin, end);

	std::vector<float> ret;

	for (auto i: vstrings) {
		ret.push_back(std::stof(i));
	}

	return ret;

}

inline int count_bits(int32_t v) {
	
	v = v - ((v >> 1) & 0x55555555);                       // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);        // temp
	return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
}

inline int count_bits(int64_t v) {

	int32_t left = (int32_t)(v);
	int32_t right = (int32_t)(v >> 32);

	left = left - ((left >> 1) & 0x55555555);                    // reuse input as temporary
	left = (left & 0x33333333) + ((left >> 2) & 0x33333333);     // temp
	left = ((left + (left >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count

	right = right - ((right >> 1) & 0x55555555);                    // reuse input as temporary
	right = (right & 0x33333333) + ((right >> 2) & 0x33333333);     // temp
	right = ((right + (right >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count

	return left + right;
}
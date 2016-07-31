#pragma once
#include <SFML/Graphics.hpp>

struct fps_counter {
public:
		fps_counter(){
				if(!f.loadFromFile("../assets/fonts/Arial.ttf")){
						std::cout << "couldn't find the fallback Arial font "
								         "in ../assets/fonts/" << std::endl;
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


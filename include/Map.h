#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <random>

class Map {
public: 
	Map(sf::Vector3i dim) {
		list = new char[dim.x * dim.y * dim.z];
		for (int i = 0; i < dim.x * dim.y * dim.x; i++) {
			list[i] = 0;
		}

        for (int x = 0; x < dim.x; x++) {
            for (int y = 0; y < dim.y; y++) {
                for (int z = 0; z < dim.z; z++) {
                    if (rand() % 100 < 1)
                        list[x + dim.x * (y + dim.z * z)] = rand() % 6;
                    else
                        list[x + dim.x * (y + dim.z * z)] = 0;
                }
            }
        }

		dimensions = dim;
		global_light = sf::Vector3f(0.2, 0.4, 1);
	}

	~Map() {
	}

	sf::Vector3i getDimensions();
	char *list;
	sf::Vector3i dimensions;

	void moveLight(sf::Vector2f in);
	sf::Vector3f global_light;

protected:

private:

};



#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>

class Map {
public: 
	Map(sf::Vector3i dim) {
		list = new char[dim.x * dim.y * dim.z];
		for (int i = 0; i < dim.x * dim.y * dim.x; i++) {
			list[i] = 0;
		}

		for (int x = 0; x < dim.x; x++) {
			for (int y = 0; y < dim.y; y++) {
				list[x + dim.x * (y + dim.z * 1)] = 1;
			}
		}
		dimensions = dim;
	}

	~Map() {
	}

	sf::Vector3i getDimensions();
	char *list;
	sf::Vector3i dimensions;

protected:

private:

};



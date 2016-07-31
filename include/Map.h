#pragma once
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>

class Map {
public: 
	Map(sf::Vector3i dim) {
		list = new char[dim.x * dim.y * dim.z];
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



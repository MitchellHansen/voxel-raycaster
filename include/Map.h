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

		for (int i = 50; i < 52; i++) {
			list[55 + dim.x * (55 + dim.z * i)] = 1;
		}

		for (int x = 0; x < dim.x; x += 2) {
			for (int y = 0; y < dim.y; y += 2) {
				list[x + dim.x * (y + dim.z * 1)] = 1;
			}
		}

		for (int x = 0; x < dim.x; x += 2) {
			for (int y = 0; y < dim.y; y += 2) {
				list[x + dim.x * (y + dim.z * 99)] = 2;
			}
		}

		for (int x = 0; x < dim.x; x += 2) {
			for (int z = 0; z < dim.z; z += 2) {
				list[x + dim.x * (99 + dim.z * z)] = 3;
			}
		}
		for (int x = 0; x < dim.x; x += 2) {
			for (int z = 0; z < dim.z; z += 2) {
				list[x + dim.x * (1 + dim.z * z)] = 4;
			}
		}


		for (int y = 0; y < dim.y; y += 2) {
			for (int z = 0; z < dim.z; z += 2) {
				list[99 + dim.x * (y + dim.z * z)] = 5;
			}
		}
		for (int y = 0; y < dim.y; y += 2) {
			for (int z = 0; z < dim.z; z += 2) {
				list[1 + dim.x * (y + dim.z * z)] = 6;
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



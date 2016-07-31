#pragma once
#include "Map.h"
#include <iostream>
#include <SFML/System/Vector3.hpp>

sf::Vector3i Map::getDimensions() {
	return dimensions;
}
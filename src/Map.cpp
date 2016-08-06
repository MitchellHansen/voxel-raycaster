#pragma once
#include "Map.h"
#include <iostream>
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Vector2.hpp>
#include "util.hpp"

sf::Vector3i Map::getDimensions() {
	return dimensions;
}

void Map::moveLight(sf::Vector2f in) {

    sf::Vector3f light_spherical = CartToSphere(global_light);

    light_spherical.y += in.y;
    light_spherical.x += in.x;

    global_light = SphereToCart(light_spherical);

    return;

}

//void Map::GenerateFloor(){
//}
















































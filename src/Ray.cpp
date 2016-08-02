#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include "Map.h"
#include <Ray.h>
#include "util.hpp"

Ray::Ray(
        Map *map,
        sf::Vector2<int> resolution,
        sf::Vector2<int> pixel,
        sf::Vector3<float> camera_position,
        sf::Vector3<float> ray_direction) {

    this->map = map;
    origin = camera_position;
    direction = ray_direction;

    dimensions = map->getDimensions();
}

sf::Color Ray::Cast() {

    // Get the cartesian direction for computing
    sf::Vector3<float> cartesian = SphereToCart(direction);

    // Setup the voxel step based on what direction the ray is pointing
    sf::Vector3<int> voxel_step(1, 1, 1);
    voxel_step.x *= (cartesian.x > 0) - (cartesian.x < 0);
    voxel_step.y *= (cartesian.y > 0) - (cartesian.y < 0);
    voxel_step.z *= (cartesian.z > 0) - (cartesian.z < 0);

    // Setup the voxel coords from the camera origin
    voxel = sf::Vector3<int>(
            (int) origin.x,
            (int) origin.y,
            (int) origin.z
    );

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
    delta_t = sf::Vector3<float>(
            fabsf((float) (1.0 / cartesian.x)),
            fabsf((float) (1.0 / cartesian.y)),
            fabsf((float) (1.0 / cartesian.z))
    );

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.

    // I think this is where the hangup is currently. It's taking the delta_t which is signed
    // and multiplying it by the voxel_step which is also signed. On top of this. Computing the
    // camera position by voxel coord is debug only so I need to do the math to account for the
    // origin being anywhere inside a voxel
    intersection_t = sf::Vector3<float>(
            delta_t.x + origin.x,
            delta_t.y + origin.y,
            delta_t.z + origin.z
    );

    int dist = 0;

    do {
        if ((intersection_t.x) < (intersection_t.y)) {
            if ((intersection_t.x) < (intersection_t.z)) {

                voxel.x += voxel_step.x;
                intersection_t.x = intersection_t.x + delta_t.x;
            } else {

                voxel.z += voxel_step.z;
                intersection_t.z = intersection_t.z + delta_t.z;
            }
        } else {
            if ((intersection_t.y) < (intersection_t.z)) {

                voxel.y += voxel_step.y;
                intersection_t.y = intersection_t.y + delta_t.y;
            } else {

                voxel.z += voxel_step.z;
                intersection_t.z = intersection_t.z + delta_t.z;
            }
        }

        // If the voxel went out of bounds
        if (voxel.z >= dimensions.z) {
            return sf::Color(0, 0, 255, 50);
        }
        if (voxel.x >= dimensions.x) {
            return sf::Color(0, 0, 255, 100);
        }
        if (voxel.y >= dimensions.x) {
            return sf::Color(0, 0, 255, 150);
        }

        if (voxel.x < 0) {
            return sf::Color(0, 255, 0, 150);
        }
        if (voxel.y < 0) {
            return sf::Color(0, 255, 0, 100);
        }
        if (voxel.z < 0) {
            return sf::Color(0, 255, 0, 50);
        }
        // If we found a voxel
        // Registers hit on non-zero

        switch (map->list[voxel.x + dimensions.x * (voxel.y + dimensions.z * voxel.z)]) {
            case 1:
                return sf::Color::Red;
            case 2:
                return sf::Color::Magenta;
            case 3:
                return sf::Color::Yellow;
            case 4:
                return sf::Color(40, 230, 96, 200);
            case 5:
                return sf::Color(80, 120, 96, 100);
            case 6:
                return sf::Color(150, 80, 220, 200);
        }
        //else if (map->list[voxel.x + dimensions.x * (voxel.y + dimensions.z * voxel.z)] != 0){
        //
        //		//TODO: Switch that assigns color on voxel data
        //		return sf::Color::Red;
        //}
        dist++;


    } while (dist < 200);

    return sf::Color::Cyan;
}




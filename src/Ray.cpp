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

    this->pixel = pixel;
    this->map = map;
    origin = camera_position;
    direction = ray_direction;
    dimensions = map->getDimensions();
}

sf::Color Ray::Cast() {

    // Setup the voxel step based on what direction the ray is pointing
    sf::Vector3<int> voxel_step(1, 1, 1);
    voxel_step.x *= (direction.x > 0) - (direction.x < 0);
    voxel_step.y *= (direction.y > 0) - (direction.y < 0);
    voxel_step.z *= (direction.z > 0) - (direction.z < 0);

    // Setup the voxel coords from the camera origin
    voxel = sf::Vector3<int>(
            floorf(origin.x),
            floorf(origin.y),
            floorf(origin.z)
    );

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
    delta_t = sf::Vector3<float>(
        fabsf(1.0f / direction.x),
        fabsf(1.0f / direction.y),
        fabsf(1.0f / direction.z)
    );

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
    intersection_t = sf::Vector3<float>(
        delta_t.x,
        delta_t.y,
        delta_t.z
    );

    int dist = 0;

    // Andrew Woo's raycasting algo
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

        // If the ray went out of bounds
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

        // If we hit a voxel
        int index = voxel.x + dimensions.x * (voxel.y + dimensions.z * voxel.z);
        switch (map->list[index]) {
            case 1:
                return sf::Color::Red;
            case 2:
                return sf::Color::Magenta;
            case 3:
                return sf::Color::Yellow;
            case 4:
                return sf::Color(80, 0, 150, 255);
            case 5:
                return sf::Color(255, 120, 255, 255);
            case 6:
                return sf::Color(150, 80, 220, 255);
        }

        dist++;


    } while (dist < 200);

    // Ray timeout color
    return sf::Color::Cyan;
}




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

    // Get the cartesian direction for computing
    sf::Vector3<float> cartesian = direction;//SphereToCart(direction);

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

    // So the way I need to do the camera is this.
    // 1.) Setup the viewplane and then store the values
    //          - Camera origin
    //          - Resolution of the view plane X, Y
    //          - Focal length to determine FOV
    //
    //  2.) For each draw. Get a copy of the view plane
    //  3.) Rotate around the X axis first, left and right
    //  4.) Then rotate alond the Y axis, up and down.
    //  5.) Make sure to limit the camera Y Rotation to 180 and -180 degrees
    //          - Rays will still go pas 180 for the amount of FOV the camera has!

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
    intersection_t = sf::Vector3<float>(
            delta_t.x + origin.x,
            delta_t.y + origin.y,
            delta_t.z + origin.z
    );

    if (pixel.y == 200){
        int i = 0;
        i++;
    }

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

        dist++;


    } while (dist < 200);

    return sf::Color::Cyan;
}




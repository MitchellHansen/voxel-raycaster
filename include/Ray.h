#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Map.h"

class Ray {

		private:

				// The Tail of the vector
			sf::Vector3<float> origin;

				// Direction / Length of the vector
				sf::Vector3<float> direction;

				// The incrementing points at which T intersects int(X, Y, Z) points
				sf::Vector3<float> intersection_t;

				// The speed at which the ray climbs.
				// Take the slope of the line (1 / cartesian.x/y/z) = delta_t.x/y/z
				sf::Vector3<float> delta_t;

				// The 3d voxel position the ray is currently at
				sf::Vector3<int> voxel;

				// Reference to the voxel map
				Map *map;

				// The dimensions of the voxel map
				sf::Vector3<int> dimensions;

		public:

			Ray(
				Map *m,
				sf::Vector2<int> resolution,
				sf::Vector2<int> pixel,
				sf::Vector3<float> camera_position,
				sf::Vector3<float> ray_direction
			);

			sf::Color Cast();
};

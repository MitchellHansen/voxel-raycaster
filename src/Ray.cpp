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
		sf::Vector3<float> ray_direction){
				
				this->map = map;
				origin = camera_position;
				direction = ray_direction;
				dimensions = map->getDimensions();
		}

sf::Color Ray::Cast(){

				// Get the cartesian direction for computing slope
				sf::Vector3<float> cartesian = SphereToCart(direction);

				// Compute the slopes
				delta_t = sf::Vector3<float>(
						(float)(1.0 / cartesian.x),
						(float)(1.0 / cartesian.y),
						(float)(1.0 / cartesian.z)
				);

				// Setup the voxel coords from the camera origin
				voxel = sf::Vector3<int>(
					(int)origin.x,
					(int)origin.y,
					(int)origin.z
					);

				// Set the first intersection to be offset by the VOXEL camera position
				intersection_t = sf::Vector3<float>(
						delta_t.x + voxel.x,
						delta_t.y + voxel.y,
						delta_t.z + voxel.z
				);

				// Setup the voxel step based on what direction the ray is pointing
				sf::Vector3<int> voxel_step(1, 1, 1);

				if (direction.x <= 0.0f || direction.x >= 3.14f) {
						voxel_step.x *= -1;
				}
				if (direction.y <= 0.0f || direction.y >= 3.14f) {
						voxel_step.y *= -1;
				}
				if (direction.z <= 0.0f || direction.z >= 3.14f) {
						voxel_step.z *= -1;
				}

				int dist = 0;
				
				do {
						if(intersection_t.x < intersection_t.y) {
								if(intersection_t.x < intersection_t.z) {
										voxel.x = voxel.x + voxel_step.x;
										intersection_t.x = intersection_t.x + delta_t.x;
								} else {
										voxel.z = voxel.z + voxel_step.z;
										intersection_t.z= intersection_t.z + delta_t.z;
								}
						} else {
								if(intersection_t.y < intersection_t.z) {
										voxel.y = voxel.y + voxel_step.y;
										intersection_t.y = intersection_t.y + delta_t.y;
								} else {
										voxel.z = voxel.z + voxel_step.z;	
										intersection_t.z = intersection_t.z + delta_t.z;
								}
						}

						// If the voxel went out of bounds
						if (voxel.x > 49 || voxel.y > 49){
								return sf::Color::Blue;;
						}
						else if (voxel.z > 49 || voxel.x < 0 || voxel.y < 0 || voxel.z < 0){
								return sf::Color::Green;
						}
						// If we found a voxel
						// Registers hit on non-zero
						else if (map->list[voxel.x, voxel.y, voxel.z] != 0){
								
								//TODO: Switch that assigns color on voxel data
								return sf::Color::Red;
						} 
						else {
								dist++;
						}

				} while(dist < 200);

				return sf::Color::Cyan;
		}

		


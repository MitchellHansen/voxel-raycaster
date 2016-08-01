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



				// Setup the voxel step based on what direction the ray is pointing
				sf::Vector3<int> voxel_step(1, 1, 1);

				//if (direction.x <= 0.0f || direction.x >= 3.14f) {
				//		voxel_step.x *= -1;
				//}
				
				// Up down
				//if (direction.y < 0.0f) {
				//	voxel_step.z *= -1;
				//}
				//if (direction.y > PI * 2 + PI / 2 ||  direction.y < -1 *PI * 2 + PI / 2) {
				//	voxel_step.x *= -1;
				//}
				// Left right
				/*if (direction.z > 1.57) {
						voxel_step.y *= -1;
						voxel_step.x *= -1;
				}*/
				//if (direction.z <= 3.14f + 1.57f && direction.z > 0.0f + 1.57f) {
				//	voxel_step.z *= -1;
				//}

				voxel_step.x *= (cartesian.x > 0) - (cartesian.x < 0);
				voxel_step.y *= (cartesian.y > 0) - (cartesian.y < 0);
				voxel_step.z *= (cartesian.z > 0) - (cartesian.z < 0);

				// Set the first intersection to be offset by the VOXEL camera position
				intersection_t = sf::Vector3<float>(
					delta_t.x * voxel_step.x + voxel.x,
					delta_t.y * voxel_step.y + voxel.y,
					delta_t.z * voxel_step.z + voxel.z
				);

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
						if (voxel.z >= dimensions.z){
							return sf::Color(0, 0, 255, 50);
						}
						if (voxel.x >= dimensions.x){
							return sf::Color(0, 0, 255, 100);
						}
						if (voxel.y >= dimensions.x){
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
	

				} while(dist < 200);

				return sf::Color::Cyan;
		}

		


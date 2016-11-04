#include "Software_Caster.h"



Software_Caster::Software_Caster()
{
}


Software_Caster::~Software_Caster()
{
}

int Software_Caster::init()
{
	return 1;
}

void Software_Caster::create_viewport(int width, int height, float v_fov, float h_fov)
{
	// CL needs the screen resolution
	sf::Vector2i view_res(width, height);

	// And an array of vectors describing the way the "lens" of our
	// camera works
	// This could be modified to make some odd looking camera lenses

	double y_increment_radians = DegreesToRadians(v_fov / view_res.y);
	double x_increment_radians = DegreesToRadians(h_fov / view_res.x);

	viewport_matrix = new sf::Vector4f[width * height * 4];

	for (int y = -view_res.y / 2; y < view_res.y / 2; y++) {
		for (int x = -view_res.x / 2; x < view_res.x / 2; x++) {

			// The base ray direction to slew from
			sf::Vector3f ray(1, 0, 0);

			// Y axis, pitch
			ray = sf::Vector3f(
				static_cast<float>(ray.z * sin(y_increment_radians * y) + ray.x * cos(y_increment_radians * y)),
				static_cast<float>(ray.y),
				static_cast<float>(ray.z * cos(y_increment_radians * y) - ray.x * sin(y_increment_radians * y))
				);


			// Z axis, yaw
			ray = sf::Vector3f(
				static_cast<float>(ray.x * cos(x_increment_radians * x) - ray.y * sin(x_increment_radians * x)),
				static_cast<float>(ray.x * sin(x_increment_radians * x) + ray.y * cos(x_increment_radians * x)),
				static_cast<float>(ray.z)
				);

			int index = (x + view_res.x / 2) + view_res.x * (y + view_res.y / 2);
			ray = Normalize(ray);

			viewport_matrix[index] = sf::Vector4f(
				ray.x,
				ray.y,
				ray.z,
				0
				);
		}
	}

	// Create the image that opencl's rays write to
	viewport_image = new sf::Uint8[width * height * 4];

	for (int i = 0; i < width * height * 4; i += 4) {

		viewport_image[i] = 255;     // R
		viewport_image[i + 1] = 255; // G
		viewport_image[i + 2] = 255; // B
		viewport_image[i + 3] = 100; // A
	}

	// Interop lets us keep a reference to it as a texture
	viewport_texture.create(width, height);
	viewport_texture.update(viewport_image);
	viewport_sprite.setTexture(viewport_texture);


}

void Software_Caster::assign_lights(std::vector<Light> lights) {
	this->lights = std::vector<Light>(lights);

	int light_count = static_cast<int>(lights.size());
}

void Software_Caster::assign_map(Old_Map * map) {
	this->map = map;
}

void Software_Caster::assign_camera(Camera * camera) {
	this->camera = camera;
}

void Software_Caster::validate() {
	// Check to make sure everything has been entered;
	if (camera == nullptr ||
		map == nullptr ||
		viewport_image == nullptr ||
		viewport_matrix == nullptr) {

		std::cout << "Raycaster.validate() failed, camera, map, or viewport not initialized";

	}
}

void Software_Caster::compute() {
	cast_rays();
}

void Software_Caster::draw(sf::RenderWindow * window) {
	viewport_texture.update(viewport_image);
	window->draw(viewport_sprite);
}

void Software_Caster::cast_rays()
{

	int i = 0;
	for (int y = 0; y < viewport_resolution.y; y++) {
		for (int x = 0; x < viewport_resolution.x; x++) {

			int id = i;
			sf::Vector2i pixel = { id % viewport_resolution.x, id / viewport_resolution.x };
			
			// 4f 3f ??
			sf::Vector4f ray_dir = viewport_matrix[pixel.x + viewport_resolution.x * pixel.y];

			ray_dir = sf::Vector4f(
				ray_dir.z * sin(camera->get_direction().x) + ray_dir.x * cos(camera->get_direction().x),
				ray_dir.y,
				ray_dir.z * cos(camera->get_direction().x) - ray_dir.x * sin(camera->get_direction().x),
				0
			);

			ray_dir = sf::Vector4f(
				ray_dir.x * cos(camera->get_direction().y) - ray_dir.y * sin(camera->get_direction().y),
				ray_dir.x * sin(camera->get_direction().y) + ray_dir.y * cos(camera->get_direction().y),
				ray_dir.z,
				0
			);

			// Setup the voxel step based on what direction the ray is pointing
			sf::Vector3i voxel_step = sf::Vector3i(
				static_cast<int>(1 * (abs(ray_dir.x) / ray_dir.x)), 
				static_cast<int>(1 * (abs(ray_dir.y) / ray_dir.y)),
				static_cast<int>(1 * (abs(ray_dir.z) / ray_dir.z))
			);
			
			// Setup the voxel coords from the camera origin
			sf::Vector3i voxel = sf::Vector3i(
				static_cast<int>(camera->get_position().x),
				static_cast<int>(camera->get_position().y),
				static_cast<int>(camera->get_position().z)
			);

			// Delta T is the units a ray must travel along an axis in order to
			// traverse an integer split
			sf::Vector3f delta_t = sf::Vector3f(
				fabs(1.0f / ray_dir.x),
				fabs(1.0f / ray_dir.y),
				fabs(1.0f / ray_dir.z)
			);

			// offset is how far we are into a voxel, enables sub voxel movement
			sf::Vector3f offset = sf::Vector3f(
				(camera->get_position().x - floor(camera->get_position().x)) * voxel_step.x,
				(camera->get_position().y - floor(camera->get_position().y)) * voxel_step.y, 
				(camera->get_position().z - floor(camera->get_position().z)) * voxel_step.z
			);

			// Intersection T is the collection of the next intersection points
			// for all 3 axis XYZ.
			sf::Vector3f intersection_t = sf::Vector3f(
				delta_t.x * offset.x,
				delta_t.y * offset.y, 
				delta_t.z * offset.z
			);

			// for negative values, wrap around the delta_t, rather not do this
			// component wise, but it doesn't appear to want to work
			if (intersection_t.x < 0) {
				intersection_t.x += delta_t.x;
			}
			if (intersection_t.y < 0) {
				intersection_t.y += delta_t.y;
			}
			if (intersection_t.z < 0) {
				intersection_t.z += delta_t.z;
			}

			// use a ghetto ass rng to give rays a "fog" appearance 
			sf::Vector2i randoms = { 3, 14 };
			int seed = randoms.x + id;
			int t = seed ^ (seed << 11);
			int result = randoms.y ^ (randoms.y >> 19) ^ (t ^ (t >> 8));

			int max_dist = 800 + result % 50;
			int dist = 0;

			sf::Vector3i mask = { 0, 0, 0 };

			// Andrew Woo's raycasting algo
			do {

				if ((intersection_t.x) < (intersection_t.y)) {
					if ((intersection_t.x) < (intersection_t.z)) {

						mask.x = 1;
						voxel.x += voxel_step.x;
						intersection_t.x = intersection_t.x + delta_t.x;
					}
					else {

						mask.z = 1;
						voxel.z += voxel_step.z;
						intersection_t.z = intersection_t.z + delta_t.z;
					}
				}
				else {
					if ((intersection_t.y) < (intersection_t.z)) {

						mask.y = 1;
						voxel.y += voxel_step.y;
						intersection_t.y = intersection_t.y + delta_t.y;
					}
					else {

						mask.z = 1;
						voxel.z += voxel_step.z;
						intersection_t.z = intersection_t.z + delta_t.z;
					}
				}


				// If the ray went out of bounds
				sf::Vector3i overshoot = sf::Vector3i(
					voxel.x <= map->getDimensions().x,
					voxel.y <= map->getDimensions().y, 
					voxel.z <= map->getDimensions().z
				);

				sf::Vector3i undershoot = sf::Vector3i(
					voxel.x > 0,
					voxel.y > 0,
					voxel.z > 0
				);

				if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0 || undershoot.x == 0 || undershoot.y == 0) {
					viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
					return;
				}
				if (undershoot.z == 0) {
					viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
					return;
				}

				// If we hit a voxel
				//int index = voxel.x * (*map_dim).y * (*map_dim).z + voxel.z * (*map_dim).z + voxel.y;
				// Why the off by one on voxel.y?
				int index = voxel.x + map->getDimensions().x * (voxel.y + map->getDimensions().z * (voxel.z - 1));
				int voxel_data = map->get_voxel_data()[index];

				if (voxel_data != 0) {
					switch (voxel_data) {
					case 255:
						viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
						return;
					case 2:
						viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
						return;
					case 3:
						viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
						return;
					case 4:
						viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
						return;
					case 5:
						viewport_image[x + viewport_resolution.x * y] = (0, 255, 255, 255);
					case 6:
						viewport_image[x + viewport_resolution.x * y] = (255, 0, 255, 255);
						return;
					default:
						//write_imagef(image, pixel, (float4)(.30, .2550, .2550, 255.00));
						continue;
					}
				}

				dist++;
			} while (dist < max_dist);

			viewport_image[x + viewport_resolution.x * y] = (255, 0, 0, 255);
			return;
		}
	}
}

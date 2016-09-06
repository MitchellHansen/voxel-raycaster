

uint4 white_light(uint4 input, float3 light, int3 mask) {

	input.w = input.w + acos(
		dot(
			normalize(light),
			normalize(fabs(convert_float3(mask)))
			)
		) * 50;

	return (input);

}

__kernel void min_kern(
        global char* map,
        global int3* map_dim,
        global int2* resolution,
        global float3* projection_matrix,
        global float3* cam_dir,
        global float3* cam_pos,
		global float* lights,
		global int* light_count,
        __write_only image2d_t image
){

    // Get the pixel position of this worker
    size_t id = get_global_id(0);
    int2 pixel = {id % resolution->x, id / resolution->x};


    // Slew the ray into it's correct position based on the view matrix's starting position
    // and the camera's current direction

    float3 ray_dir = projection_matrix[pixel.x + resolution->x * pixel.y];

    // Yaw
    ray_dir = (float3)(
            ray_dir.z * sin(cam_dir->y) + ray_dir.x * cos(cam_dir->y),
            ray_dir.y,
            ray_dir.z * cos(cam_dir->y) - ray_dir.x * sin(cam_dir->y)
    );

    // Pitch
    ray_dir = (float3)(
          ray_dir.x * cos(cam_dir->z) - ray_dir.y * sin(cam_dir->z),
          ray_dir.x * sin(cam_dir->z) + ray_dir.y * cos(cam_dir->z),
          ray_dir.z
    );

    // Setup the voxel step based on what direction the ray is pointing
    int3 voxel_step = {1, 1, 1};
	voxel_step *= (ray_dir > 0) - (ray_dir < 0);

    // Setup the voxel coords from the camera origin
	int3 voxel = convert_int3(*cam_pos);

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
	float3 delta_t = fabs(1.0f / ray_dir);

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
	float3 intersection_t = delta_t;

    // Create a psuedo random number for view fog
	int2 randoms = { 3, 14 };
	uint seed = randoms.x + id;
	uint t = seed ^ (seed << 11);
	uint result = randoms.y ^ (randoms.y >> 19) ^ (t ^ (t >> 8));

    // Distance a ray can travel before it terminates
	int max_dist = 200 + result % 50;
	int dist = 0;

    // Bitmask to keep track of which axis was tripped
	int3 mask = { 0, 0, 0 };

    // Andrew Woo's raycasting algo
    do {

        // Non-branching test of the lowest delta_t value
		mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);

		// Based on the result increment the voxel and intersection
		intersection_t += delta_t * fabs(convert_float3(mask.xyz));
		voxel.xyz += voxel_step.xyz * mask.xyz;

        // If the ray went out of bounds
		int3 overshoot = voxel <= *map_dim;
		int3 undershoot = voxel > 0;

        // "Sky"
		if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0 || undershoot.x == 0 || undershoot.y == 0){
			write_imageui(image, pixel, (uint4)(135, 206, 235, 255));
			return;
		}

		// "Water"
		if (undershoot.z == 0) {
			write_imageui(image, pixel, (uint4)(64, 164, 223, 255));
			return;
		}

        // If we hit a voxel
        int index = voxel.x + map_dim->x * (voxel.y + map_dim->z * voxel.z);
        int voxel_data = map[index];

		if (voxel_data != 0) {
			switch (voxel_data) {
			case 1:
				write_imageui(image, pixel, (uint4)(50, 0, 0, 255));
				return;
			case 2:
				write_imageui(image, pixel, (uint4)(0, 50, 40, 255));
				return;
			case 3:
				write_imageui(image, pixel, (uint4)(0, 0, 50, 255));
				return;
			case 4:
				write_imageui(image, pixel, (uint4)(25, 0, 25, 255));
				return;
			case 5:
				//write_imageui(image, pixel, (uint4)(200, 200, 200, 255));
				write_imageui(image, pixel, white_light((uint4)(44, 176, 55, 100), (float3)(lights[7], lights[8], lights[9]), mask));
				return;
			case 6:
				write_imageui(image, pixel, (uint4)(30, 80, 10, 255));
				return;
			}
		}

        dist++;
    } while (dist < max_dist);

    write_imageui(image, pixel, (uint4)(135, 206, 235, 255));
    return;
}
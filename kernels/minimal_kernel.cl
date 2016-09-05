__kernel void min_kern(
        global char* map,
        global int3* map_dim,
        global int2* resolution,
        global float3* projection_matrix,
        global float3* cam_dir,
        global float3* cam_pos,
        __write_only image2d_t image
){

    size_t id = get_global_id(0);
    int2 pixel = {id % resolution->x, id / resolution->x};
    float3 ray_dir = projection_matrix[pixel.x + resolution->x * pixel.y];

    ray_dir = (float3)(
            ray_dir.z * sin(cam_dir->y) + ray_dir.x * cos(cam_dir->y),
            ray_dir.y,
            ray_dir.z * cos(cam_dir->y) - ray_dir.x * sin(cam_dir->y)
    );

    ray_dir = (float3)(
            ray_dir.x * cos(cam_dir->z) - ray_dir.y * sin(cam_dir->z),
            ray_dir.x * sin(cam_dir->z) + ray_dir.y * cos(cam_dir->z),
            ray_dir.z
    );

    // Setup the voxel step based on what direction the ray is pointing
    int3 voxel_step = {1, 1, 1};
    voxel_step.x *= (ray_dir.x > 0) - (ray_dir.x < 0);
    voxel_step.y *= (ray_dir.y > 0) - (ray_dir.y < 0);
    voxel_step.z *= (ray_dir.z > 0) - (ray_dir.z < 0);

    // Setup the voxel coords from the camera origin
    int3 voxel = {
            floor(cam_pos->x),
            floor(cam_pos->y),
            floor(cam_pos->z)
    };

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
    float3 delta_t = {
            fabs(1.0f / ray_dir.x),
            fabs(1.0f / ray_dir.y),
            fabs(1.0f / ray_dir.z)
    };

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
    float3 intersection_t = {
            delta_t.x,
            delta_t.y,
            delta_t.z
    };

	int2 randoms = { 3, 7 };
	uint seed = randoms.x + id;
	uint t = seed ^ (seed << 11);
	uint result = randoms.y ^ (randoms.y >> 19) ^ (t ^ (t >> 8));

	int max_dist = 500 + result % 50;
    int dist = 0;
    int face = -1;
    // X:0, Y:1, Z:2


	int3 mask = { 0, 0, 0 };

    // Andrew Woo's raycasting algo
    do {

		mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		float3 thing = delta_t * fabs(convert_float3(mask.xyz));
		intersection_t += delta_t * fabs(convert_float3(mask.xyz));
		voxel.xyz += voxel_step.xyz * mask.xyz;



        // If the ray went out of bounds
		int3 overshoot = voxel.xyz <= map_dim->xyz;
		int3 undershoot = voxel > 0;


		
		if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0){
			write_imagef(image, pixel, (float4)(.50 * abs(overshoot.x), .50 * abs(overshoot.y), .50 * abs(overshoot.z), 1));
			return;
		}
		if (undershoot.x == 0 || undershoot.y == 0 || undershoot.z == 0) {
			write_imagef(image, pixel, (float4)(.1 * abs(undershoot.x), .80 * abs(undershoot.y), .20 * abs(undershoot.z), 1));
			return;
		}

        // If we hit a voxel
        int index = voxel.x + map_dim->x * (voxel.y + map_dim->z * voxel.z);
        int voxel_data = map[index];



		if (voxel_data != 0) {
			switch (voxel_data) {
			case 1:
				write_imagef(image, pixel, (float4)(.50, .00, .00, 1));
				return;
			case 2:
				write_imagef(image, pixel, (float4)(.00, .50, .40, 1.00));
				//if (id == 249000)
				   // printf("%i\n", voxel_data);
				return;
			case 3:
				write_imagef(image, pixel, (float4)(.00, .00, .50, 1.00));
				return;
			case 4:
				write_imagef(image, pixel, (float4)(.25, .00, .25, 1.00));
				return;
			case 5:
				write_imagef(image, pixel, (float4)(.10, .30, .80, 1.00));
				return;
			case 6:
				write_imagef(image, pixel, (float4)(.30, .80, .10, 1.00));
				return;
			}
		}

        dist++;
    } while (dist < max_dist);

    write_imagef(image, pixel, (float4)(.00, .00, .00, .00));
    return;
}
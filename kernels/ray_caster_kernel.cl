

float4 white_light(float4 input, float3 light, int3 mask) {

	input.w = input.w + acos(
		dot(
			normalize(light),
			normalize(fabs(convert_float3(mask)))
			)
		) / 2;

	return input;

}

// {r, g, b, i, x, y, z, x', y', z'}

float4 cast_light_rays(float3 ray_origin, global float* lights, global int* light_count) {


}

__kernel void min_kern(
        global char* map,
        global int3* map_dim,
        global int2* resolution,
        global float3* projection_matrix,
        global float2* cam_dir,
        global float3* cam_pos,
		global float* lights,
		global int* light_count,
        __write_only image2d_t image
){

    size_t id = get_global_id(0);
    int2 pixel = {id % (*resolution).x, id / (*resolution).x};
    float3 ray_dir = projection_matrix[pixel.x + (*resolution).x * pixel.y];

    ray_dir = (float3)(
            ray_dir.z * sin((*cam_dir).x) + ray_dir.x * cos((*cam_dir).x),
            ray_dir.y,
            ray_dir.z * cos((*cam_dir).x) - ray_dir.x * sin((*cam_dir).x)
    );

    ray_dir = (float3)(
            ray_dir.x * cos((*cam_dir).y) - ray_dir.y * sin((*cam_dir).y),
            ray_dir.x * sin((*cam_dir).y) + ray_dir.y * cos((*cam_dir).y),
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

	// offset is how far we are into a voxel, enables sub voxel movement
	float3 offset = ((*cam_pos) - floor(*cam_pos)) * convert_float3(voxel_step);
    
	//offset.x += delta_t.x * convert_float((voxel_step.x < 0));
	//offset -= delta_t * floor(offset / delta_t);

	// Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
	float3 intersection_t = delta_t * offset;

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

	int2 randoms = { 3, 14 };
	uint seed = randoms.x + id;
	uint t = seed ^ (seed << 11);
	uint result = randoms.y ^ (randoms.y >> 19) ^ (t ^ (t >> 8));

	int max_dist = 800 + result % 50;
	int dist = 0;

	int3 mask = { 0, 0, 0 };

    // Andrew Woo's raycasting algo
    do {

		mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(mask.xyz));
		voxel.xyz += voxel_step.xyz * mask.xyz;

        // If the ray went out of bounds
		int3 overshoot = voxel <= *map_dim;
		int3 undershoot = voxel > 0;

		if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0 || undershoot.x == 0 || undershoot.y == 0){
			write_imagef(image, pixel, (float4)(.73, .81, .89, 1.0));
			return;
		}
		if (undershoot.z == 0) {
			write_imagef(image, pixel, (float4)(.14, .30, .50, 1.0));
			return;
		}

        // If we hit a voxel
		//int index = voxel.x * (*map_dim).y * (*map_dim).z + voxel.z * (*map_dim).z + voxel.y;
		// Why the off by one on voxel.y?
        int index = voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z-1));
        int voxel_data = map[index];

		if (voxel_data != 0) {
			switch (voxel_data) {
			case 1:
				write_imagef(image, pixel, (float4)(.50, .00, .00, 1));
				return;
			case 2:
				write_imagef(image, pixel, (float4)(.00, .50, .40, 1.00));
				return;
			case 3:
				write_imagef(image, pixel, (float4)(.00, .00, .50, 1.00));
				return;
			case 4:
				write_imagef(image, pixel, (float4)(.25, .00, .25, 1.00));
				return;
			case 5:
				//write_imagef(image, pixel, (float4)(.25, .00, .25, 1.00));
			write_imagef(image, pixel, white_light((float4)(.25, .32, .14, 0.2), (float3)(lights[7], lights[8], lights[9]), mask));
				//cast_light_rays(voxel, lights, light_count)
				return;
			case 6:
				write_imagef(image, pixel, (float4)(.30, .80, .10, 1.00));
				return;
			default:
				write_imagef(image, pixel, (float4)(.30, .80, .10, 1.00));
				return;
			}
		}

        dist++;
    } while (dist < max_dist);

    write_imagef(image, pixel, (float4)(.73, .81, .89, 1.0));
    return;
}
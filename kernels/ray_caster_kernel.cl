

float4 white_light(float4 input, float3 light, int3 mask) {

	input.w = input.w + acos(
		dot(
			normalize(light),
			normalize(convert_float3(mask * (-mask)))
			)
		) / 2;

	return input;

}

bool cast_light_intersection_ray(
	global char* map,
	global int3* map_dim,
	 float3 ray_dir,
	 float3 ray_pos,
	global float* lights,
	global int* light_count 
	
	){

	// Setup the voxel step based on what direction the ray is pointing
	int3 voxel_step = { 1, 1, 1 };
	voxel_step *= (ray_dir > 0) - (ray_dir < 0);

	// Setup the voxel coords from the camera origin
	int3 voxel = convert_int3(ray_pos);

	// Delta T is the units a ray must travel along an axis in order to
	// traverse an integer split
	float3 delta_t = fabs(1.0f / ray_dir);

	// offset is how far we are into a voxel, enables sub voxel movement
	float3 offset = ((ray_pos) - floor(ray_pos)) * convert_float3(voxel_step);

	// Intersection T is the collection of the next intersection points
	// for all 3 axis XYZ.
	float3 intersection_t = delta_t *offset;

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

	// Hard cut-off for how far the ray can travel
	int max_dist = 800;
	int dist = 0;

	int3 face_mask = { 0, 0, 0 };

	// Andrew Woo's raycasting algo
	do {

		// If we hit a voxel
		int index = voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z));
		int voxel_data = map[index];

		if (voxel_data != 0) {
			return true;
		}

		// Fancy no branch version of the logic step
		face_mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(face_mask.xyz));
		voxel.xyz += voxel_step.xyz * face_mask.xyz;

		// If the ray went out of bounds
		int3 overshoot = voxel <= *map_dim;
		int3 undershoot = voxel > 0;

		if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0 || undershoot.x == 0 || undershoot.y == 0) {
			return false;
		}
		if (undershoot.z == 0) {
			return false;
		}



		dist++;

	} while (dist < 700);

	return false;
}

float4 view_light(float4 in_color, float3 light, float3 view, int3 mask) {
	
	float diffuse = max(dot(normalize(convert_float3(mask)), normalize(light)), 0.0f);
	in_color += diffuse * 0.5;

	if (dot(light, normalize(convert_float3(mask))) > 0.0)
	{
		float3 halfwayVector = normalize(normalize(light) + normalize(view));
		float specTmp = max(dot(normalize(convert_float3(mask)), halfwayVector), 0.0f);
		in_color += pow(specTmp, 1.0f) * 0.01;
	}

	//float3 halfwayDir = normalize(normalize(view) + normalize(light));
	//float spec = pow(max(dot(normalize(convert_float3(mask)), halfwayDir), 0.0f), 32.0f);
	in_color += 0.02;
	return in_color;
}







//  0  1  2  3  4  5  6  7   8   9
// {r, g, b, i, x, y, z, x', y', z'}

float4 cast_light_rays(
	float3 eye_direction, 
	float3 ray_origin, 
	float4 voxel_color, 
	float3 voxel_normal, 
	global float* lights, 
	global int* light_count) {

	// set the ray origin to be where the initial ray intersected the voxel
	// which side z, and the x and y position

	float ambient_constant = 0.5;
	float intensity = 0;

	for (int i = 0; i < *light_count; i++) {

		float distance = sqrt(
			pow(lights[10 * i + 4] - ray_origin.x, 2) +
			pow(lights[10 * i + 5] - ray_origin.y, 2) +
			pow(lights[10 * i + 6] - ray_origin.z, 2));

		if (distance > 50)
			continue;

		float3 light_direction = (lights[10 * i + 7], lights[10 * i + 8], lights[10 * i + 9]);
		float c = 10.0;

		//if (dot(light_direction, voxel_normal) > 0.0) {
			float3 halfwayVector = normalize(light_direction + eye_direction);
			float dot_prod = dot(voxel_normal, halfwayVector);
			float specTmp = max((float)dot_prod, 0.0f);
			intensity += pow(specTmp, c);
		//}
	}

	if (get_global_id(0) == 1037760) { 
		//printf("%f", intensity);
		voxel_color = (float4)(1.0, 1.0, 1.0, 1.0);
		return voxel_color;
	}

	voxel_color.w *= intensity;
	voxel_color.w += ambient_constant;

	return voxel_color;

	//	for every light
	//
	//		check if the light is within falloff distance
	//		every unit, light halfs
	//
	//		if it is, cast a ray to that light and check for collisions.
	//			if ray exits voxel volume, assume unobstructed
	//			
	//			if ray intersects a voxel, dont influence the voxel color
	//		
	//			if it does
}

int rand(int* seed) // 1 <= *seed < m
{
	int const a = 16807; //ie 7**5
	int const m = 2147483647; //ie 2**31-1

	*seed = ((*seed) * a) % m;
	return(*seed);
}



__kernel void raycaster(
        global char* map,
        global int3* map_dim,
        global int2* resolution,
        global float3* projection_matrix,
        global float2* cam_dir,
        global float3* cam_pos,
		global float* lights,
		global int* light_count,
        __write_only image2d_t image,
		global int* seed_memory
){

	int global_id = get_global_id(0);
	
	// Get and set the random seed from seed memory
	int seed = seed_memory[global_id];
	int random_number = rand(&seed);
	seed_memory[global_id] = seed;

	// Get the pixel on the viewport, and find the view matrix ray that matches it
    int2 pixel = { global_id % (*resolution).x, global_id / (*resolution).x};
    float3 ray_dir = projection_matrix[pixel.x + (*resolution).x * pixel.y];

	//if (pixel.x == 960 && pixel.y == 540) {
	//	write_imagef(image, pixel, (float4)(0.00, 1.00, 0.00, 1.00));
	//	return;
	//}

	// Pitch
	ray_dir = (float3)(
		ray_dir.z * sin((*cam_dir).x) + ray_dir.x * cos((*cam_dir).x),
		ray_dir.y,
		ray_dir.z * cos((*cam_dir).x) - ray_dir.x * sin((*cam_dir).x)
		);

	// Yaw
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

	// Hard cut-off for how far the ray can travel
	int max_dist = 800;
	int dist = 0;

	
	int3 face_mask = { 0, 0, 0 };
	float4 fog_color = { 0.73, 0.81, 0.89, 0.8 };
	float4 voxel_color = (float4)(0.50, 0.0, 0.50, 0.1);
	float4 overshoot_color = { 0.25, 0.48, 0.52, 0.8 };
	

	// Andrew Woo's raycasting algo
    do {

		// Fancy no branch version of the logic step
		face_mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(face_mask.xyz));
		voxel.xyz += voxel_step.xyz * face_mask.xyz;

        // If the ray went out of bounds
		int3 overshoot = voxel <= *map_dim;
		int3 undershoot = voxel > 0;

		if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0 || undershoot.x == 0 || undershoot.y == 0){
			write_imagef(image, pixel, white_light(mix(fog_color, overshoot_color, 1.0 - max(dist / 700.0f, (float)0)), (float3)(lights[7], lights[8], lights[9]), face_mask));
			return;
		}
		if (undershoot.z == 0) {
			write_imagef(image, pixel, white_light(mix(fog_color, overshoot_color, 1.0 - max(dist / 700.0f, (float)0)), (float3)(lights[7], lights[8], lights[9]), face_mask));
			return;
		}
		
        // If we hit a voxel
        int index = voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z));
        int voxel_data = map[index];

		if (voxel_data != 0) {

			//	write_imagef(image, pixel, (float4)(0.90, 0.00, 0.40, 0.9));

			if (voxel_data == 6) {
				voxel_color = (float4)(0.0, 0.239, 0.419, 0.3);
			} 
			else if (voxel_data == 5) {
				voxel_color = (float4)(0.25, 0.52, 0.30, 0.1);
			}


			if (cast_light_intersection_ray(
				map,
				map_dim,
				(float3)(lights[4], lights[5], lights[6]) - (convert_float3(voxel)),
				(convert_float3(voxel) - convert_float3(face_mask * voxel_step)),
				lights,
				light_count
				)) {
					
				write_imagef(image, pixel, voxel_color);
				//write_imagef(image, pixel, voxel_color);
				return;
			}

			write_imagef(
				image,
				pixel,
				view_light(
					voxel_color,
					(convert_float3(voxel) + offset) - (float3)(lights[4], lights[5], lights[6]),
					(convert_float3(voxel) + offset) - (*cam_pos),
					face_mask * voxel_step
					)
				);

			return;


		}

        dist++;

    } while (dist / 700.0f < 1);
	//dist < max_dist
	write_imagef(image, pixel, white_light(mix(fog_color, (float4)(0.40, 0.00, 0.40, 0.2), 1.0 - max(dist / 700.0f, (float)0)), (float3)(lights[7], lights[8], lights[9]), face_mask));
    //write_imagef(image, pixel, (float4)(.73, .81, .89, 1.0));
    return;
}
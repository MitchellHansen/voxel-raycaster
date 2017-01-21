
// Naive incident ray light
float4 white_light(float4 input, float3 light, int3 mask) {

	input.w = input.w + acos(
		dot(
			normalize(light),
			normalize(convert_float3(mask * (-mask)))
			)
		) / 8;

	return input;

}


// Phong + diffuse lighting function for g

//  0  1  2  3  4  5  6  7   8   9
// {r, g, b, i, x, y, z, x', y', z'}


float4 view_light(float4 in_color, float3 light, float3 view, int3 mask) {

	float diffuse = max(dot(normalize(convert_float3(mask)), normalize(light)), 0.0f);
	in_color += diffuse * 0.2;

	if (dot(light, normalize(convert_float3(mask))) > 0.0)
	{
		float3 halfwayVector = normalize(normalize(light) + normalize(view));
		float specTmp = max(dot(normalize(convert_float3(mask)), halfwayVector), 0.0f);
		in_color += pow(specTmp, 1.0f) * 0.5;
	}

	//in_color += 0.02;
	return in_color;
}


int rand(int* seed) // 1 <= *seed < m
{
	int const a = 16807; //ie 7**5
	int const m = 2147483647; //ie 2**31-1

	*seed = ((*seed) * a) % m;
	return(*seed);
}

float DistanceBetweenPoints(float3 a, float3 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

// =================================== Boolean ray intersection ============================
// =========================================================================================

bool cast_light_intersection_ray(
	global char* map,
	global int3* map_dim,
	 float3 ray_dir,
	 float3 ray_pos,
	global float* lights,
	global int* light_count 
	
	){

	float distance_to_light = DistanceBetweenPoints(ray_pos, (float3)(lights[4], lights[5], lights[6]));

	// Setup the voxel step based on what direction the ray is pointing
	int3 voxel_step = { 1, 1, 1 };
	voxel_step *= (ray_dir > 0) - (ray_dir < 0);

	// Setup the voxel coords from the camera origin
	int3 voxel = convert_int3(ray_pos);

	// Delta T is the units a ray must travel along an axis in order to
	// traverse an integer split
	float3 delta_t = fabs(1.0f / ray_dir);
	
	// offset is how far we are into a voxel, enables sub voxel movement
	float3 offset = ((ray_pos)-floor(ray_pos)) * convert_float3(voxel_step);

	// Intersection T is the collection of the next intersection points
	// for all 3 axis XYZ.
	float3 intersection_t = delta_t *offset;

	// for negative values, wrap around the delta_t
	intersection_t += delta_t * -convert_float3(isless(intersection_t, 0));

	int3 face_mask = { 0, 0, 0 };

	// Andrew Woo's raycasting algo
	do {

		// Fancy no branch version of the logic step
		face_mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(face_mask.xyz));
		voxel.xyz += voxel_step.xyz * face_mask.xyz;

		// If the ray went out of bounds
		int3 overshoot = voxel < *map_dim;
		int3 undershoot = voxel >= 0;

		if (any(overshoot  == (int3)(0, 0, 0)) ||
			any(undershoot == (int3)(0, 0, 0))) {
			return false;
		}

		// If we hit a voxel
		int index = voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z));
		int voxel_data = map[index];

		if (voxel_data != 0)
			return true;

	//} while (any(isless(intersection_t, (float3)(distance_to_light - 1))));
	} while (intersection_t.x < distance_to_light - 1 ||
		     intersection_t.y < distance_to_light - 1 || 
		     intersection_t.z < distance_to_light - 1 );
	
	return false;
}


// ====================================== Raycaster entry point =====================================
// ==================================================================================================

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
		global int* seed_memory,
		__read_only image2d_t texture_atlas,
		global int2 *atlas_dim,
		global int2 *tile_dim
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

	// for negative values, wrap around the delta_t
	intersection_t += delta_t * -convert_float3(isless(intersection_t, 0));
	

	// Hard cut-off for how far the ray can travel
	int max_dist = 800;
	int dist = 0;

	
	int3 face_mask = { 0, 0, 0 };
	float4 fog_color = { 0.73, 0.81, 0.89, 0.8 };
	float4 voxel_color = (float4)(0.50, 0.0, 0.50, 0.1);
	float4 overshoot_color = { 0.25, 0.48, 0.52, 0.8 };
	float4 overshoot_color_2 = { 0.25, 0.1, 0.52, 0.8 };
	

	// Andrew Woo's raycasting algo
    do {

		// Fancy no branch version of the logic step
		face_mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(face_mask.xyz));
		voxel.xyz += voxel_step.xyz * face_mask.xyz;

        // If the ray went out of bounds
		int3 overshoot = voxel < *map_dim;
		int3 undershoot = voxel >= 0;

		if (overshoot.x == 0 || overshoot.y == 0 || overshoot.z == 0 || undershoot.x == 0 || undershoot.y == 0){
			write_imagef(image, pixel, white_light(mix(fog_color, overshoot_color, 1.0 - max(dist / 700.0f, (float)0)), (float3)(lights[7], lights[8], lights[9]), face_mask));
			return;
		}
		if (undershoot.z == 0) {
			write_imagef(image, pixel, white_light(mix(fog_color, overshoot_color_2, 1.0 - max(dist / 700.0f, (float)0)), (float3)(lights[7], lights[8], lights[9]), face_mask));
			return;
		}
		
        // If we hit a voxel
        int index = voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z));
        int voxel_data = map[index];

		if (voxel_data != 0) {

			// Determine where on the 2d plane the ray intersected

			float3 face_position = (float)(0);
			float2 texture_position = (float)(0);


			// First determine the percent of the way the ray is towards the next intersection_t
			// in relation to the xyz position on the plane
			if (face_mask.x == -1) {

				float z_percent = (intersection_t.z - (intersection_t.x - delta_t.x)) / delta_t.z;
				float y_percent = (intersection_t.y - (intersection_t.x - delta_t.x)) / delta_t.y;

				// Since we intersected face x, we know that we are at the face (1.0)
				// Not entirely sure what is causing the 1.0 vs 1.001 rendering bug
				face_position = (float3)(1.001f, y_percent, z_percent);
				texture_position = (float2)(y_percent, z_percent);
			}
			else if (face_mask.y == -1) {

				float x_percent = (intersection_t.x - (intersection_t.y - delta_t.y)) / delta_t.x;
				float z_percent = (intersection_t.z - (intersection_t.y - delta_t.y)) / delta_t.z;

				face_position = (float3)(x_percent, 1.001f, z_percent);
				texture_position = (float2)(x_percent, z_percent);
			}

			else if (face_mask.z == -1) {

				float x_percent = (intersection_t.x - (intersection_t.z - delta_t.z)) / delta_t.x;
				float y_percent = (intersection_t.y - (intersection_t.z - delta_t.z)) / delta_t.y;
				
				face_position = (float3)(x_percent, y_percent, 1.001f);
				texture_position = (float2)(x_percent, y_percent);

			}

			// We now need to account for the ray wanting to skip the axis in which
			// it flips its sign

			// TODO: improve this

			if (ray_dir.x > 0) {
				face_position.x = -face_position.x + 1;
				texture_position.x = -texture_position.x + 1.0;
			}
			//if (ray_dir.x < 0)
			//	face_position.x = face_position.x + 0;

			if (ray_dir.y > 0){
				face_position.y =  - face_position.y + 1;
				texture_position.y = -texture_position.y + 1.0;
			}

			//if (ray_dir.y < 0)
			//	face_position.y = face_position.y + 0;

			if (ray_dir.z > 0) {
				face_position.z =  - face_position.z + 1;
				texture_position.y = -texture_position.y + 1.0;
			}

			//if (ray_dir.z < 0)
			//	face_position.z = face_position.z + 0;


			// Now either use the face position to retrieve a texture sample, or
			// just a plain color for the voxel color

			if (voxel_data == 6) {
				voxel_color = (float4)(0.0, 0.239, 0.419, 0.3);
			}
			else if (voxel_data == 5) {
				float2 tile_size = convert_float2(*atlas_dim / *tile_dim);
				voxel_color = read_imagef(texture_atlas, convert_int2(texture_position * tile_size) + convert_int2((float2)(3, 0) * tile_size));
					//voxel_color = (float4)(0.25, 0.52, 0.30, 0.1);
			}
			else if (voxel_data == 1) {
				voxel_color = (float4)(0.929, 0.957, 0.027, 0.7);
			}

			// 

			if (cast_light_intersection_ray(
				map,
				map_dim,
				normalize((float3)(lights[4], lights[5], lights[6]) - (convert_float3(voxel) + face_position)),
				(convert_float3(voxel) + face_position),
				lights, 
				light_count
			)) {

				float4 ambient_color = white_light(voxel_color, (float3)(lights[4], lights[5], lights[6]), face_mask);
				write_imagef(image, pixel, ambient_color);
				return;
			}

			//  0  1  2  3  4  5  6  7   8   9
			// {r, g, b, i, x, y, z, x', y', z'}

			write_imagef(
				image,
				pixel,
				view_light(
					voxel_color,
					(convert_float3(voxel) + face_position) - (float3)(lights[4], lights[5], lights[6]),
					(convert_float3(voxel) + face_position) - (*cam_pos),
					face_mask * voxel_step
					)
				);

			return;


		}

        dist++;

    } while (dist / 700.0f < 1);


	write_imagef(image, pixel, white_light(mix(fog_color, (float4)(0.40, 0.00, 0.40, 0.2), 1.0 - max(dist / 700.0f, (float)0)), (float3)(lights[7], lights[8], lights[9]), face_mask));
    return;
}
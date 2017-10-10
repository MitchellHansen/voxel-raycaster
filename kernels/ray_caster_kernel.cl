
// =========================================================================
// ======================== INITIALIZER CONSTANTS ==========================

__constant float4 zeroed_float4 = {0.0f, 0.0f, 0.0f, 0.0f};
__constant float3 zeroed_float3 = {0.0f, 0.0f, 0.0f};
__constant float2 zeroed_float2 = {0.0f, 0.0f};
__constant int4 zeroed_int4     = {0, 0, 0, 0};
__constant int3 zeroed_int3     = {0, 0, 0};
__constant int2 zeroed_int2     = {0, 0};

// =========================================================================
// ============================ OCTREE CONSTANTS ===========================

// (X, Y, Z) mask for the idx
__constant const uchar idx_set_x_mask = 0x1;
__constant const uchar idx_set_y_mask = 0x2;
__constant const uchar idx_set_z_mask = 0x4;

__constant const uchar mask_8[8] = {
	0x1,  0x2,  0x4,  0x8,
	0x10, 0x20, 0x40, 0x80
};

// Mask for counting the previous valid bits
__constant const uchar count_mask_8[8] = {
	0x1,  0x3,  0x7,  0xF,
	0x1F, 0x3F, 0x7F, 0xFF
};

// uint64_t manipulation masks
__constant const ulong child_pointer_mask = 0x0000000000007fff;
__constant const ulong far_bit_mask = 0x8000;
__constant const ulong valid_mask = 0xFF0000;
__constant const ulong leaf_mask = 0xFF000000;
__constant const ulong contour_pointer_mask = 0xFFFFFF00000000;
__constant const ulong contour_mask = 0xFF00000000000000;

// =========================================================================
// ========================= RAYCASTER CONSTANTS ===========================

constant float4 fog_color = { 0.0f, 0.0f, 0.0f, 0.0f };
constant float4 overshoot_color = { 0.00f, 0.00f, 0.00f, 0.00f };
constant float4 overshoot_color_2 = { 0.00f, 0.00f, 0.00f, 0.00f };

// =========================================================================
// =========================================================================

// =========================================================================
// ========================= HELPER FUNCTIONS ==============================

float DistanceBetweenPoints(float3 a, float3 b) {
	return fast_distance(a, b);
}

float Distance(float3 a) {
	return fast_length(a);
}


// Phong + diffuse lighting function for g
//  0  1  2  3  4  5  6  7   8   9
// {r, g, b, i, x, y, z, x', y', z'}

float4 view_light(float4 in_color, float3 light, float4 light_color, float3 view, int3 mask) {

	if (all(light == zeroed_float3))
		return zeroed_float4;

	float d = Distance(light) / 280.0f;
	d *= d;

	float diffuse = max(dot(normalize(convert_float3(mask)), normalize(light)), 0.0f);
	in_color += diffuse * light_color * 0.5f / d;

	if (dot(light, normalize(convert_float3(mask))) > 0.0f)
	{
		// Small dots of light are caused by floating point error
		// flipping bits on the face mask and screwing up this calculation
		float3 halfwayVector = normalize(normalize(light) + normalize(view));
		float specTmp = max(dot(normalize(convert_float3(mask)), halfwayVector), 0.0f);
		in_color += pow(specTmp, 8.0f) * light_color * 0.5f / d;
	}
	if (in_color.w > 1.0f){
		in_color.xyz *= in_color.w;
	}

	return in_color;
}


int rand(int* seed) // 1 <= *seed < m
{
	int const a = 16807; //ie 7**5
	int const m = 2147483647; //ie 2**31-1

	*seed = ((*seed) * a) % m;
	return(*seed);
}

// =========================================================================
// ========================= OCTREE TRAVERSAL ==============================

bool get_oct_vox(
	int3 position,
	global ulong *octree_descriptor_buffer,
	global uint *octree_attachment_lookup_buffer,
	global ulong *octree_attachment_buffer,
	global ulong *settings_buffer
){

	// push the root node to the parent stack
	ulong current_index = *settings_buffer;
	ulong head = octree_descriptor_buffer[current_index];

	uint parent_stack_position = 0;
	ulong parent_stack[32];

	uchar scale = 0;
	uchar idx_stack[32];

	ulong current_descriptor = 0;

	bool found = false;

	parent_stack[parent_stack_position] = head;

	// Set our initial dimension and the position at the corner of the oct to keep track of our position
	int dimension = 64;
	int3 quad_position = zeroed_int3;

	// While we are not at the required resolution
	//		Traverse down by setting the valid/leaf mask to the subvoxel
	//		Check to see if it is valid
	//			Yes?
	//				Check to see if it is a leaf
	//				No? Break
	//				Yes? Scale down to the next hierarchy, push the parent to the stack
	//
	//			No?
	//				Break
	while (dimension > 1) {

		// So we can be a little bit tricky here and increment our
		// array index that holds our masks as we build the idx.
		// Adding 1 for X, 2 for Y, and 4 for Z
		int mask_index = 0;

		// Do the logic steps to find which sub oct we step down into
		if (position.x >= (dimension / 2) + quad_position.x) {

			// Set our voxel position to the (0,0) of the correct oct
			quad_position.x += (dimension / 2);

			// increment the mask index and mentioned above
			mask_index += 1;

			// Set the idx to represent the move
			idx_stack[scale] |= idx_set_x_mask;

		}
		if (position.y >= (dimension / 2) + quad_position.y) {

			quad_position.y |= (dimension / 2);
			mask_index += 2;
			idx_stack[scale] |= idx_set_y_mask;

		}
		if (position.z >= (dimension / 2) + quad_position.z) {

			quad_position.z += (dimension / 2);
			mask_index += 4;
			idx_stack[scale] |= idx_set_z_mask;
		}

		// Check to see if we are on a valid oct
		if ((head >> 16) & mask_8[mask_index]) {

			// Check to see if it is a leaf
			if ((head >> 24) & mask_8[mask_index]) {

				// If it is, then we cannot traverse further as CP's won't have been generated
				found = true;
				return found;
			}

			// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
			scale++;
			dimension /= 2;

			// Count the number of valid octs that come before and add it to the index to get the position
			// Negate it by one as it counts itself
			int count = popcount((uchar)(head >> 16) & count_mask_8[mask_index]) - 1;

			// access the element at which head points to and then add the specified number of indices
			// to get to the correct child descriptor
			current_index = current_index + (head & child_pointer_mask) + count;
			head = octree_descriptor_buffer[current_index];

			// Increment the parent stack position and put the new oct node as the parent
			parent_stack_position++;
			parent_stack[parent_stack_position] = head;

		}
		else {
			// If the oct was not valid, then no CP's exists any further
			// This implicitly says that if it's non-valid then it must be a leaf!!

			// It appears that the traversal is now working but I need
			// to focus on how to now take care of the end condition.
			// Currently it adds the last parent on the second to lowest
			// oct CP. Not sure if thats correct
			found = 0;
			return found;
		}
	}

	found = 1;
	return found;
}

// =========================================================================
// ========================= RAYCASTER ENTRY ===============================

__kernel void raycaster(
	global char* map,
	constant int3* map_dim,
	constant int2* resolution,
	global float3* projection_matrix,
	global float2* cam_dir,
	global float3* cam_pos,
	global float* lights,
	global int* light_count,
	__write_only image2d_t image,
	__read_only image2d_t texture_atlas,
	constant int2 *atlas_dim,
	constant int2 *tile_dim,
	global ulong *octree_descriptor_buffer,
	global uint *octree_attachment_lookup_buffer,
	global ulong *octree_attachment_buffer,
	global ulong *settings_buffer
){

	// Get the pixel on the viewport, and find the view matrix ray that matches it
	int2 pixel = (int2)(get_global_id(0), get_global_id(1));
    float3 ray_dir = projection_matrix[pixel.x + (*resolution).x * pixel.y];

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

	if (any(ray_dir == zeroed_float3))
		return;

	// Setup the voxel step based on what direction the ray is pointing
    int3 voxel_step = {1, 1, 1};
	voxel_step *= (ray_dir > 0) - (ray_dir < 0);

    // Setup the voxel coords from the camera origin
	int3 voxel = convert_int3(*cam_pos);

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
	float3 delta_t = fabs(1.0f / ray_dir);

	// Intersection T is the collection of the next intersection points
	// for all 3 axis XYZ. We take the full positive cardinality when
	// subtracting the floor, so we must transfer the sign over from
	// the voxel step
	float3 intersection_t = delta_t * ((*cam_pos) - ceil(*cam_pos)) * convert_float3(voxel_step);

	// When we transfer the sign over, we get the correct direction of
	// the offset, but we merely transposed over the value instead of mirroring
	// it over the axis like we want. So here, isless returns a boolean if intersection_t
	// is less than 0 which dictates whether or not we subtract the delta which in effect
	// mirrors the offset
	intersection_t -= delta_t * convert_float3(isless(intersection_t, 0));

	int distance_traveled = 0;
	int max_distance = 700;
	uint bounce_count = 0;
	int3 face_mask = { 0, 0, 0 };
	int voxel_data = 0;
	float3 face_position = zeroed_float3;
	float4 voxel_color= zeroed_float4;
	float2 tile_face_position = zeroed_float2;
	float3 sign = zeroed_float3;
	float4 color_accumulator = zeroed_float4;
	float fog_distance = 0.0f;

	bool shadow_ray = false;

	// Andrew Woo's raycasting algo
    while (distance_traveled < max_distance && bounce_count < 4) {

		// Fancy no branch version of the logic step
		face_mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(face_mask.xyz));
		voxel.xyz += voxel_step.xyz * face_mask.xyz;

		// Test for out of bounds contions, add fog
		if (any(voxel >= *map_dim) || any(voxel < 0)){
			voxel.xyz -= voxel_step.xyz * face_mask.xyz;
			color_accumulator = mix(fog_color, voxel_color, 1.0f - max(distance_traveled / 700.0f, 0.0f));
			color_accumulator.w *= 4;
			break;
		}

        // If we hit a voxel
		if (voxel.x < 64 && voxel.y < 64 && voxel.z < 64){
		 	if (get_oct_vox(
		 		voxel,
		 		octree_descriptor_buffer,
		 		octree_attachment_lookup_buffer,
		 		octree_attachment_buffer,
		 		settings_buffer
		 		)){
		 			voxel_data = 5;
		 		} else {
		 			voxel_data = 0;
		 		}
		} else {
			voxel_data = map[voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z))];
		}


		if (voxel_data != 0) {

			// Determine where on the 2d plane the ray intersected
			face_position = zeroed_float3;
			tile_face_position = zeroed_float2;
			sign = (1.0f, 1.0f, 1.0f);

			// First determine the percent of the way the ray is towards the next intersection_t
			// in relation to the xyz position on the plane
			if (face_mask.x == -1) {

				sign.x *= -1.0;

				// the next intersection for this plane - the last intersection of the passed plane / delta of this plane
				// basically finds how far in on the other 2 axis we are when the ray traversed the plane
				float z_percent = (intersection_t.z - (intersection_t.x - delta_t.x)) / delta_t.z;
				float y_percent = (intersection_t.y - (intersection_t.x - delta_t.x)) / delta_t.y;

				// Since we intersected face x, we know that we are at the face (1.0)
				// I think the 1.001f rendering bug is the ray thinking it's within the voxel
				// even though it's sitting on the very edge
				face_position = (float3)(1.0001f, y_percent, z_percent);
				tile_face_position = face_position.yz;
			}
			else if (face_mask.y == -1) {

				sign.y *= -1.0;
				float x_percent = (intersection_t.x - (intersection_t.y - delta_t.y)) / delta_t.x;
				float z_percent = (intersection_t.z - (intersection_t.y - delta_t.y)) / delta_t.z;
				face_position = (float3)(x_percent, 1.0001f, z_percent);
				tile_face_position = face_position.xz;
			}

			else if (face_mask.z == -1) {

				sign.z *= -1.0;
				float x_percent = (intersection_t.x - (intersection_t.z - delta_t.z)) / delta_t.x;
				float y_percent = (intersection_t.y - (intersection_t.z - delta_t.z)) / delta_t.y;
				face_position = (float3)(x_percent, y_percent, 1.0001f);
				tile_face_position = face_position.xy;

			}

			// Because the raycasting process is agnostic to the quadrant
			// it's working in, we need to transpose the sign over to the face positions.
			// If we don't it will think that it is always working in the (1, 1, 1) quadrant
			// and will just "copy" the quadrant. This includes shadows as they use the face_position
			// in order to cast the intersection ray!!

			face_position.x = select((face_position.x), (-face_position.x + 1.0f), (int)(ray_dir.x > 0));
			tile_face_position.x = select((tile_face_position.x), (-tile_face_position.x + 1.0f), (int)(ray_dir.x < 0));

			if (ray_dir.y > 0){
				face_position.y =  -face_position.y + 1;
			} else {
				tile_face_position.x = 1.0 - tile_face_position.x;

				// We run into the Hairy ball problem, so we need to define
				// a special case for the zmask
				if (face_mask.z == -1) {
					tile_face_position.x = 1.0f - tile_face_position.x;
					tile_face_position.y = 1.0f - tile_face_position.y;
				}
			}

			face_position.z = select((face_position.z), (-face_position.z + 1.0f), (int)(ray_dir.z > 0));
			tile_face_position.y = select((tile_face_position.y), (-tile_face_position.y + 1.0f), (int)(ray_dir.z < 0));

			// Now we detect what type of of voxel we intersected and decide whether
			// to bend the ray, send out a light intersection ray, or add texture color

			// TEXTURE HIT + SHADOW REDIRECTION
			if (voxel_data == 5 && !shadow_ray){

				shadow_ray = true;
				voxel_color.xyz += (float3)read_imagef(
						 texture_atlas,
						 convert_int2(tile_face_position * convert_float2(*atlas_dim / *tile_dim)) +
						 convert_int2((float2)(3, 0) * convert_float2(*atlas_dim / *tile_dim))
				).xyz/2;

				color_accumulator = view_light(
							voxel_color,
							(convert_float3(voxel) + face_position) - (float3)(lights[4], lights[5], lights[6]),
							(float4)(lights[0], lights[1], lights[2], lights[3]),
							(convert_float3(voxel) + face_position) - (*cam_pos),
							face_mask * voxel_step
				);

				fog_distance = distance_traveled;
				max_distance = distance_traveled + DistanceBetweenPoints(convert_float3(voxel), (float3)(lights[4], lights[5], lights[6]));


				float3 hit_pos = convert_float3(voxel) + face_position;
				ray_dir = normalize((float3)(lights[4], lights[5], lights[6]) - hit_pos);
				if (any(ray_dir == zeroed_float3))
					return;

				voxel -= voxel_step * face_mask;
				voxel_step = ( 1, 1, 1 ) * ((ray_dir > 0) - (ray_dir < 0));

				delta_t = fabs(1.0f / ray_dir);
				intersection_t = delta_t * ((hit_pos)-floor(hit_pos)) * convert_float3(voxel_step);
				intersection_t += delta_t * -convert_float3(isless(intersection_t, 0));

			// REFLECTION
			} else if (voxel_data == 6 && !shadow_ray) {

				voxel_color.xyz += (float3)read_imagef(
						 texture_atlas,
						 convert_int2(tile_face_position * convert_float2(*atlas_dim / *tile_dim)) +
						 convert_int2((float2)(3, 4) * convert_float2(*atlas_dim / *tile_dim))
				).xyz/4;

				voxel_color.w -= 0.0f;
				//max_distance += 200;


				float3 hit_pos = convert_float3(voxel) + face_position;
				ray_dir *= sign;
				if (any(ray_dir == zeroed_float3))
					return;

				voxel -= voxel_step * face_mask;
				voxel_step = ( 1, 1, 1 );
				voxel_step *= (ray_dir > 0) - (ray_dir < 0);

				//voxel = convert_int3(hit_pos);

				delta_t = fabs(1.0f / ray_dir);
				intersection_t = delta_t * ((hit_pos)-floor(hit_pos)) * convert_float3(voxel_step);
				intersection_t += delta_t * -convert_float3(isless(intersection_t, 0));

				bounce_count += 1;

			// SHADOW RAY HIT
			} else {
				color_accumulator = 0;
				break;
			}
		}

		// At the bottom of the while loop, add one to the distance ticker
		distance_traveled++;
    }
	color_accumulator = mix(fog_color, color_accumulator, 1.0f - max(fog_distance / 700.0f, 0.0f));
	write_imagef(
		image,
		pixel,
		color_accumulator
	);

    return;
}


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
__constant const uchar3 idx_set_mask = {0x1, 0x2, 0x4};

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

// Phong + diffuse lighting function for g
//  0  1  2  3  4  5  6  7   8   9
// {r, g, b, i, x, y, z, x', y', z'}

float4 view_light(float4 in_color, float3 light, float4 light_color, float3 view, int3 mask) {

	if (all(light == zeroed_float3))
		return zeroed_float4;

	float d = fast_length(light) * 0.01f;
	d *= d;

	float diffuse = max(dot(normalize(convert_float3(mask)), normalize(light)), 0.1f);
	float specular = 0.0f;

	if (diffuse > 0.0f)	{
		// Small dots of light are caused by floating point error
		// flipping bits on the face mask and screwing up this calculation
		float3 halfwayVector = normalize(normalize(light) + normalize(view));
		float specTmp = max(dot(normalize(convert_float3(mask)), halfwayVector), 0.0f);
		specular = pow(specTmp, 1.0f);
	}

	in_color += diffuse * light_color + specular * light_color / d;
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

struct TraversalState {

	int parent_stack_position;
	ulong parent_stack[10];
	ulong parent_stack_index[10];

	uchar scale;
	uchar idx_stack[10];

	ulong current_descriptor;
	ulong current_descriptor_index;

	int3 oct_pos;

	// ====== DEBUG =======
	char found;

};

bool get_oct_vox(
	int3 position,
	global ulong *octree_descriptor_buffer,
	global uint *octree_attachment_lookup_buffer,
	global ulong *octree_attachment_buffer,
	global ulong *settings_buffer
){

	struct TraversalState ts;

	// push the root node to the parent stack
	ts.current_descriptor_index = *settings_buffer;
	ts.current_descriptor = octree_descriptor_buffer[ts.current_descriptor_index];
	ts.scale = 0;
	ts.found = false;
	ts.parent_stack[ts.scale] = ts.current_descriptor;

	// Set our initial dimension and the position at the corner of the oct to keep track of our position
	int dimension = OCTDIM;
	ts.oct_pos = zeroed_int3;

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
		ts.idx_stack[ts.scale] = 0;

		//	Do the logic steps to find which sub oct we step down into
		if (position.x >= (dimension / 2) + ts.oct_pos.x) {

			// Set our voxel position to the (0,0) of the correct oct
			ts.oct_pos.x += (dimension / 2);

			// Set the idx to represent the move
			ts.idx_stack[ts.scale] |= idx_set_x_mask;

		}
		if (position.y >= (dimension / 2) + ts.oct_pos.y) {

			ts.oct_pos.y += (dimension / 2);
			ts.idx_stack[ts.scale] |= idx_set_y_mask;

		}
		if (position.z >= (dimension / 2) + ts.oct_pos.z) {

			ts.oct_pos.z += (dimension / 2);
			ts.idx_stack[ts.scale] |= idx_set_z_mask;
		}

		int mask_index = ts.idx_stack[ts.scale];

		// Check to see if we are on a valid oct
		if ((ts.current_descriptor >> 16) & mask_8[mask_index]) {

			// Check to see if it is a leaf
			if ((ts.current_descriptor >> 24) & mask_8[mask_index]) {

				// If it is, then we cannot traverse further as CP's won't have been generated
				ts.found = true;
				return ts.found;
			}

			// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
			ts.scale++;
			dimension /= 2;

			// Count the number of valid octs that come before and add it to the index to get the position
			// Negate it by one as it counts itself
			int count = popcount((uchar)(ts.current_descriptor >> 16) & count_mask_8[mask_index]) - 1;

			// access the far point at which the head points too. Determine it's value, and add
			// a count of the valid bits to the index
			if (far_bit_mask & octree_descriptor_buffer[ts.current_descriptor_index]) {
				int far_pointer_index = ts.current_descriptor_index + (ts.current_descriptor & child_pointer_mask);
				ts.current_descriptor_index = octree_descriptor_buffer[far_pointer_index] + count;
			}
			// access the element at which head points to and then add the specified number of indices
			// to get to the correct child descriptor
			else {
				ts.current_descriptor_index = ts.current_descriptor_index + (ts.current_descriptor & child_pointer_mask) + count;
			}
			ts.current_descriptor = octree_descriptor_buffer[ts.current_descriptor_index];


			ts.parent_stack[ts.scale] = ts.current_descriptor;

		}
		else {
			// If the oct was not valid, then no CP's exists any further
			// This implicitly says that if it's non-valid then it must be a leaf!!

			// It appears that the traversal is now working but I need
			// to focus on how to now take care of the end condition.
			// Currently it adds the last parent on the second to lowest
			// oct CP. Not sure if thats correct
			ts.found = 0;
			return ts.found;
		}
	}

	ts.found = 1;
	return ts.found;
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
	int3 voxel = convert_int3_rtn(*cam_pos);

	//voxel = voxel + convert_int3(*cam_pos < 0.0f);
    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
	float3 delta_t = fabs(1.0f / ray_dir);

	// Intersection T is the collection of the next intersection points
	// for all 3 axis XYZ. We take the full positive cardinality when
	// subtracting the floor, so we must transfer the sign over from
	// the voxel step

	float3 offset = delta_t * ((*cam_pos) - ceil(*cam_pos));
	float3 intersection_t = offset* convert_float3(voxel_step);

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
    while (distance_traveled < max_distance && bounce_count < 2) {

		// Fancy no branch version of the logic step
		face_mask = intersection_t.xyz <= min(intersection_t.yzx, intersection_t.zxy);
		intersection_t += delta_t * fabs(convert_float3(face_mask.xyz));
		voxel.xyz += voxel_step.xyz * face_mask.xyz;

// =======================================================================================================================================
// =======================================================================================================================================
// =======================================================================================================================================

		// uchar prev_val = traversal_state.idx_stack[traversal_state.scale];
		// uint8_t this_face_mask = 0;
		//
		// // Check the voxel face that we traversed
		// // and increment the idx in the idx stack
		// if (face_mask.x) {
		// 	this_face_mask = Octree::idx_set_x_mask;
		// }
		// else if (face_mask.y) {
		// 	this_face_mask = Octree::idx_set_y_mask;
		// }
		// else if (face_mask.z) {
		// 	this_face_mask = Octree::idx_set_z_mask;
		// }
		//
		// traversal_state.idx_stack[traversal_state.scale] ^= this_face_mask;
		//
		// // Mask index is the 1D index'd value of the idx for interaction with the valid / leaf masks
		// int mask_index = traversal_state.idx_stack[traversal_state.scale];
		//
		// // Whether or not the next oct we want to enter in the current CD's valid mask is 1 or 0
		// bool is_valid = false;
		//
		// // TODO: Rework this logic so we don't have this bodgy if
		// if (mask_index > prev_val)
		// 	is_valid = (traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & mask_8[mask_index];
		//
		// // Check to see if the idx increased or decreased
		// // If it decreased
		// //		Pop up the stack until the oct that the idx flip is valid and we landed on a valid oct
		// while (mask_index < prev_val || !is_valid) {
		//
		// 	jump_power *= 2;
		//
		// 	// Keep track of the 0th edge of out current oct
		// 	traversal_state.oct_pos.x = floor(voxel.x / 2) * jump_power;
		// 	traversal_state.oct_pos.y = floor(voxel.y / 2) * jump_power;
		// 	traversal_state.oct_pos.z = floor(voxel.z / 2) * jump_power;
		//
		// 	// Clear and pop the idx stack
		// 	traversal_state.idx_stack[traversal_state.scale] = 0;
		//
		// 	// Scale is now set to the oct above. Be wary of this
		// 	traversal_state.scale--;
		//
		// 	// Update the prev_val for our new idx
		// 	prev_val = traversal_state.idx_stack[traversal_state.scale];
		//
		// 	// Clear and pop the parent stack, maybe off by one error?
		// 	traversal_state.parent_stack_index[traversal_state.parent_stack_position] = 0;
		// 	traversal_state.parent_stack[traversal_state.parent_stack_position] = 0;
		// 	traversal_state.parent_stack_position--;
		//
		// 	// Set the current CD to the one on top of the stack
		// 	traversal_state.current_descriptor =
		// 		traversal_state.parent_stack[traversal_state.parent_stack_position];
		//
		// 	// Apply the face mask to the new idx for the while check
		// 	traversal_state.idx_stack[traversal_state.scale] ^= this_face_mask;
		//
		// 	// Get the mask index of the new idx and check the valid status
		// 	mask_index = traversal_state.idx_stack[traversal_state.scale];
		// 	is_valid = (traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & mask_8[mask_index];
		// }
		//
		// // At this point parent_stack[position] is at the CD of an oct with a valid oct at the leaf indicated by the current
		// // idx in the idx stack scale
		//
		// // While we haven't bottomed out and the oct we're looking at is valid
		// while (jump_power > 1 && is_valid) {
		//
		// 	// If all went well and we found a valid non-leaf oct then we will traverse further down the hierarchy
		// 	traversal_state.scale++;
		// 	jump_power /= 2;
		//
		// 	// Count the number of valid octs that come before and add it to the index to get the position
		// 	// Negate it by one as it counts itself
		// 	int count = count_bits((uint8_t)(traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & count_mask_8[mask_index]) - 1;
		//
		// 	// If this CD had the far bit set
		// 	if (far_bit_mask & descriptor_buffer[traversal_state.parent_stack_index[traversal_state.parent_stack_position]]) {
		//
		// 		// access the far point at which the head points too. Determine it's value, and add
		// 		// the count of the valid bits in the current CD to the index
		// 		uint64_t far_pointer_index =
		// 			traversal_state.parent_stack_index[traversal_state.parent_stack_position] + // current index +
		// 			(traversal_state.parent_stack[traversal_state.parent_stack_position] & child_pointer_mask); // the relative prt to the far ptr
		//
		// 		// Get the absolute ptr from the far ptr and add the count to get the CD that we want
		// 		traversal_state.parent_stack_index[traversal_state.parent_stack_position + 1] = descriptor_buffer[far_pointer_index] + count;
		// 	}
		// 	// If this CD doesn't have the far bit set, access the element at which head points to
		// 	// and then add the specified number of indices to get to the correct child descriptor
		// 	else {
		// 		traversal_state.parent_stack_index[traversal_state.parent_stack_position + 1] =
		// 			traversal_state.parent_stack_index[traversal_state.parent_stack_position] + // The current index to this CD
		// 			(traversal_state.parent_stack[traversal_state.parent_stack_position] & child_pointer_mask) + count; // The relative dist + the number of bits that were valid
		// 	}
		//
		// 	// Now that we have the index set we can increase our parent stack position to the next level and
		// 	// retrieve the value of its CD
		// 	traversal_state.parent_stack_position++;
		// 	traversal_state.parent_stack[traversal_state.parent_stack_position] = descriptor_buffer[traversal_state.parent_stack_index[traversal_state.parent_stack_position]];
		//
		// 	// Unlike the single shot DFS, it makes a bit more sense to have this at the tail of the while loop
		// 	// Do the logic steps to find which sub oct we step down into
		// 	if (voxel.x >= (jump_power / 2) + traversal_state.oct_pos.x) {
		//
		// 		// Set our voxel position to the (0,0) of the correct oct
		// 		traversal_state.oct_pos.x += (jump_power / 2);
		//
		// 		// Set the idx to represent the move
		// 		traversal_state.idx_stack[traversal_state.scale] |= idx_set_x_mask;
		//
		// 	}
		// 	if (voxel.y >= (jump_power / 2) + traversal_state.oct_pos.y) {
		//
		// 		traversal_state.oct_pos.y += (jump_power / 2);
		// 		traversal_state.idx_stack[traversal_state.scale] |= idx_set_y_mask;
		// 	}
		// 	if (voxel.z >= (jump_power / 2) + traversal_state.oct_pos.z) {
		//
		// 		traversal_state.oct_pos.z += (jump_power / 2);
		// 		traversal_state.idx_stack[traversal_state.scale] |= idx_set_z_mask;
		// 	}
		//
		// 	// Update the mask index with the new voxel we walked down to, and then check it's valid status
		// 	mask_index = traversal_state.idx_stack[traversal_state.scale];
		// 	is_valid = (traversal_state.parent_stack[traversal_state.parent_stack_position] >> 16) & mask_8[mask_index];
		//
		// }

// =======================================================================================================================================
// =======================================================================================================================================
// =======================================================================================================================================

		// Test for out of bounds contions, add fog
		if (any(voxel >= *map_dim) || any(voxel < 0)){
			voxel.xyz -= voxel_step.xyz * face_mask.xyz;
			color_accumulator = mix(fog_color, voxel_color, 1.0f - max(distance_traveled / 700.0f, 0.0f));
			color_accumulator.w *= 4;
			break;
		}

		constant int vox_dim = OCTDIM;

//        If we hit a voxel
		// if (voxel.x < (*map_dim).x && voxel.y < (*map_dim).x && voxel.z < (*map_dim).x){
		//  	if (get_oct_vox(
		//  		voxel,
		//  		octree_descriptor_buffer,
		//  		octree_attachment_lookup_buffer,
		//  		octree_attachment_buffer,
		//  		settings_buffer
		//  		)){
		//  			voxel_data = 5;
		//  		} else {
		//  			voxel_data = 0;
		//  		}
		// } else {
			voxel_data = map[voxel.x + (*map_dim).x * (voxel.y + (*map_dim).z * (voxel.z))];
		//}



		if (voxel_data == 5 || voxel_data == 6) {
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
				face_position = (float3)(1.00001f, y_percent, z_percent);
				tile_face_position = face_position.yz;
			}
			else if (face_mask.y == -1) {

				sign.y *= -1.0;
				float x_percent = (intersection_t.x - (intersection_t.y - delta_t.y)) / delta_t.x;
				float z_percent = (intersection_t.z - (intersection_t.y - delta_t.y)) / delta_t.z;
				face_position = (float3)(x_percent, 1.00001f, z_percent);
				tile_face_position = face_position.xz;
			}

			else if (face_mask.z == -1) {

				sign.z *= -1.0;
				float x_percent = (intersection_t.x - (intersection_t.z - delta_t.z)) / delta_t.x;
				float y_percent = (intersection_t.y - (intersection_t.z - delta_t.z)) / delta_t.y;
				face_position = (float3)(x_percent, y_percent, 1.00001f);
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

			// TEXTURE HIT + SHADOW RAY REDIRECTION
			if (voxel_data == 5 && !shadow_ray){

				shadow_ray = true;
				voxel_color.xyz += (float3)read_imagef(
						 texture_atlas,
						 convert_int2(tile_face_position * convert_float2(*atlas_dim / *tile_dim)) +
						 convert_int2((float2)(5, 0) * convert_float2(*atlas_dim / *tile_dim))
				).xyz/2;

				color_accumulator = view_light(
							voxel_color,
							(convert_float3(voxel) + face_position) - (float3)(lights[4], lights[5], lights[6]),
							(float4)(lights[0], lights[1], lights[2], lights[3]),
							(convert_float3(voxel) + face_position) - (*cam_pos),
							face_mask * voxel_step
				);

				fog_distance = distance_traveled;
				max_distance = distance_traveled + fast_distance(convert_float3(voxel), (float3)(lights[4], lights[5], lights[6]));

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

				float3 hit_pos = convert_float3(voxel) + face_position;
				ray_dir *= sign;
				if (any(ray_dir == zeroed_float3))
					return;

				voxel -= voxel_step * face_mask;
				voxel_step = ( 1, 1, 1 );
				voxel_step *= (ray_dir > 0) - (ray_dir < 0);

				delta_t = fabs(1.0f / ray_dir);
				intersection_t = delta_t * ((hit_pos)-floor(hit_pos)) * convert_float3(voxel_step);
				intersection_t += delta_t * -convert_float3(isless(intersection_t, 0));

				bounce_count += 1;

			// SHADOW RAY HIT
			} else {
				color_accumulator.w = 0.1f;
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

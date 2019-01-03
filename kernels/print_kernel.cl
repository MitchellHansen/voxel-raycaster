

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

        size_t id = get_global_id(0);

//	if (id == 0) {
//
//		printf("MAP: %i, %i, %i, %i", map[0], map[1], map[2], map[3]);
//		printf("MAP_DIMENSIONS: %i, %i, %i", map_dim[0].x, map_dim[0].y, map_dim[0].z);
//		printf("RESOLUTION: %i, %i", resolution[0].x, resolution[0].y);
//		printf("PROJECTION_MATRIX: %f, %f, %f", projection_matrix[0].x, projection_matrix[0].y, projection_matrix[0].z);
//		printf("CAMERA_DIRECTION: %f, %f", cam_dir[0].x, cam_dir[0].y);
//		printf("CAMERA_POSITION: %f, %f, %f", cam_pos[0].x, cam_pos[0].y, cam_pos[0].z);
//		printf("LIGHTS: %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", lights[0], lights[1], lights[2], lights[3], lights[4], lights[5], lights[6], lights[7], lights[8], lights[9]);
//		printf("LIGHT_COUNT: %i", light_count);
//		
//
//
//	}

	return;

}

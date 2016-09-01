__kernel void min_kern(
        global char* in,
        global char* map,
        global int3* map_dim,
        global int2* resolution,
        global float3* projection_matrix,
        global float3* cam_dir,
        global float3* cam_pos
){

    size_t id = get_global_id(0);

    //printf("%i %c -- ", id, map[id]);
    //printf("%i, %i, %i\n", map_dim->x, map_dim->y, map_dim->z);
    //printf("\n%i\nX: %f\nY: %f\nZ: %f\n", id, projection_matrix[id].x, projection_matrix[id].y, projection_matrix[id].z);
    //printf("%f, %f, %f\n", cam_dir->x, cam_dir->y, cam_dir->z);
    //printf("%f, %f, %f\n", cam_pos->x, cam_pos->y, cam_pos->z);

}
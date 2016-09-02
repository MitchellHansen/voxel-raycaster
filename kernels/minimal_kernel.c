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

    float4 black = (float4)(.49, .68, .81, 1);

    int2 pixelcoord = (int2) (id, id);

    write_imagef(image, pixelcoord, black);


    //printf("%i %i -- ", id, map[id]);
    //printf("%i, %i, %i\n", map_dim->x, map_dim->y, map_dim->z);
    //printf("\n%i\nX: %f\nY: %f\nZ: %f\n", id, projection_matrix[id].x, projection_matrix[id].y, projection_matrix[id].z);
    //printf("%f, %f, %f\n", cam_dir->x, cam_dir->y, cam_dir->z);
    //printf("%f, %f, %f\n", cam_pos->x, cam_pos->y, cam_pos->z);




}
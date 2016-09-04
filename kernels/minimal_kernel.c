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
    //printf("%i === %f, %f, %f\n", id, ray_dir.x, ray_dir.y, ray_dir.z);

    // Y axis, pitch
    //ray_dir.x = ray_dir.z * sin(cam_dir->y) + ray_dir.x * cos(cam_dir->y);
    //ray_dir.y = ray_dir.y;
    //ray_dir.z = ray_dir.z * cos(cam_dir->y) - ray_dir.x * sin(cam_dir->y);


    ray_dir = (float3)(
            ray_dir.z * sin(cam_dir->y) + ray_dir.x * cos(cam_dir->y),
            ray_dir.y,
            ray_dir.z * cos(cam_dir->y) - ray_dir.x * sin(cam_dir->y)
    );

    // Z axis, yaw
    //ray_dir.x = ray_dir.x * cos(cam_dir->z) - ray_dir.y * sin(cam_dir->z);
    //ray_dir.y = ray_dir.x * sin(cam_dir->z) + ray_dir.y * cos(cam_dir->z);
    //ray_dir.z = ray_dir.z;

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
            floorf(cam_pos->x),
            floorf(cam_pos->y),
            floorf(cam_pos->z)
    };

    // Delta T is the units a ray must travel along an axis in order to
    // traverse an integer split
    float3 delta_t = {
            fabsf(1.0f / ray_dir.x),
            fabsf(1.0f / ray_dir.y),
            fabsf(1.0f / ray_dir.z)
    };

    // Intersection T is the collection of the next intersection points
    // for all 3 axis XYZ.
    float3 intersection_t = {
            delta_t.x,
            delta_t.y,
            delta_t.z
    };


    int dist = 0;
    int face = -1;
    // X:0, Y:1, Z:2

    // Andrew Woo's raycasting algo
    do {
        if ((intersection_t.x) < (intersection_t.y)) {
            if ((intersection_t.x) < (intersection_t.z)) {

                face = 0;
                voxel.x += voxel_step.x;
                intersection_t.x = intersection_t.x + delta_t.x;
            } else {

                face = 2;
                voxel.z += voxel_step.z;
                intersection_t.z = intersection_t.z + delta_t.z;
            }
        } else {
            if ((intersection_t.y) < (intersection_t.z)) {

                face = 1;
                voxel.y += voxel_step.y;
                intersection_t.y = intersection_t.y + delta_t.y;
            } else {

                face = 2;
                voxel.z += voxel_step.z;
                intersection_t.z = intersection_t.z + delta_t.z;
            }
        }

        // If the ray went out of bounds
        if (voxel.z >= map_dim->z) {
            write_imagef(image, pixel, (float4)(.5, .50, .00, 1));
            return;
        }
        if (voxel.x >= map_dim->x) {
            write_imagef(image, pixel, (float4)(.00, .00, .99, 1));
            return;
        }
        if (voxel.y >= map_dim->x) {
            write_imagef(image, pixel, (float4)(.00, .44, .00, 1));
            return;
        }

        if (voxel.x < 0) {
            write_imagef(image, pixel, (float4)(.99, .00, .99, 1));
            return;
        }
        if (voxel.y < 0) {
            write_imagef(image, pixel, (float4)(.99, .99, .00, 1));
            return;
        }
        if (voxel.z < 0) {
            write_imagef(image, pixel, (float4)(.00, .99, .99, 1));
            return;
        }

        // If we hit a voxel
        int index = voxel.x + map_dim->x * (voxel.y + map_dim->z * voxel.z);
        int voxel_data = map[index];

        if (id == 240000)
            printf("%i, %i, %i\n", voxel.x, voxel.y, voxel.z);

        switch (voxel_data) {
            case 1:
                write_imagef(image, pixel, (float4)(.50, .00, .00, 1));
                return;
            case 2:
                write_imagef(image, pixel, (float4)(.00, .50, .40, 1.00));
                if (id == 249000)
                    printf("%i\n", voxel_data);
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

        dist++;
    } while (dist < 600);


    write_imagef(image, pixel, (float4)(.00, .00, .00, .00));
    return;

    //printf("%i %i -- ", id, map[id]);
    //printf("%i, %i, %i\n", map_dim->x, map_dim->y, map_dim->z);
    //printf("\n%i\nX: %f\nY: %f\nZ: %f\n", id, projection_matrix[id].x, projection_matrix[id].y, projection_matrix[id].z);
    //printf("%f, %f, %f\n", cam_dir->x, cam_dir->y, cam_dir->z);
    //printf("%f, %f, %f\n", cam_pos->x, cam_pos->y, cam_pos->z);

}
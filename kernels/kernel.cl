// global  :  local  :  constant  :  private

// Function arguments of type image2d_t, image3d_t, image2d_array_t, image1d_t, image1d_buffer_t,
// and image1d_array_t refer to image memory objects allocated in the **global** address space.

// http://downloads.ti.com/mctools/esd/docs/opencl/memory/buffers.html

// Open CL C
// https://www.fixstars.com/en/opencl/book/OpenCLProgrammingBook/opencl-c/

__kernel void hello(
    global int2* resolution,
    global char* map,
    global float3* projection_matrix,
    global float3* cam_dir,
    global float3* cam_pos,
    global image2d_t* canvas) {

    printf("%s\n", "this is a test string\n");



    const int MAX_RAY_STEPS = 64;

    // The pixel coord we are at
    int2 screenPos = (int2)(get_global_id(0) % resolution->x, get_global_id(0) / resolution->x);

    // The X and Y planes
    //float3 cameraPlaneU = vec3(1.0, 0.0, 0.0)

    // Y being multiplied by the aspect ratio, usually around .5-6ish;
    //cl_float3 cameraPlaneV = vec3(0.0, 1.0, 0.0) * iResolution.y / iResolution.x;

    // So this is how they do that ray aiming! hah this is so tiny
    // (camera direction) + (pixel.x * the X plane) + (product of pixel.y * Y plane)
    // Oh all it's doing is adding the x and y coords of the pixel to the camera direction vector, interesting

    //cl_float3 rayDir = cameraDir + screenPos.x * cameraPlaneU + screenPos.y * cameraPlaneV;

    // the origin of the ray
    // So the sign thing is for the up and down motion

    //cl_float3 rayPos = vec3(0.0, 2.0 * sin(iGlobalTime * 2.7), -12.0);

    // Ah, and here is where it spins around the center axis
    // So it looks like its applying a function to rotate the x and z axis
    //rayPos.xz = rotate2d(rayPos.xz, iGlobalTime);
    //rayDir.xz = rotate2d(rayDir.xz, iGlobalTime);

    // Just an intvec of out coords
    //ivec3 mapPos = ivec3(floor(rayPos));

    // I think this is the delta t value
    // the magnitude of the vector divided by the rays direction. Not sure what the aim of that is
    // The ray direction might always be normalized, so that would be the dame as my delta_T
    //vec3 deltaDist = abs(vec3(length(rayDir)) / rayDir);

    // The steps are the signs of the ray direction
    //ivec3 rayStep = ivec3(sign(rayDir));

    //  ithe sign of the rays direction
    //  *
    //	Convert map position to a floating point vector and take away the ray position
    //  +
    //  the sign of the rays direction by 0.5
    //  +
    //  0.5
    //  Now multyply everything by 0.5
    //vec3 sideDist = (sign(rayDir) * (vec3(mapPos) - rayPos) + (sign(rayDir) * 0.5) + 0.5) * deltaDist;

    // A byte mask
    //bvec3 mask;

    // repeat until the max steps
    //for (int i = 0; i < MAX_RAY_STEPS; i++) {

            // If there is a voxel at the map position, continue?
            //if (getVoxel(mapPos))
            //        break;

            //
            // find which is smaller
            // y ? z --> x`
            // z ? x --> y`
            // x ? y --> z`
            //
            // find which os is less or equal
            // x` ? x --> x
            // y` ? y --> y
            // z` ? z --> z

            // Now find which ons is
            //mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));


            // Originally he used a component wise
            /*bvec3 b1 = lessThan(sideDist.xyz, sideDist.yzx);
            bvec3 b2 = lessThanEqual(sideDist.xyz, sideDist.zxy);
            mask.x = b1.x && b2.x;
            mask.y = b1.y && b2.y;
            mask.z = b1.z && b2.z;*/
            //Would've done mask = b1 && b2 but the compiler is making me do it component wise.

            //All components of mask are false except for the corresponding largest component
            //of sideDist, which is the axis along which the ray should be incremented.

            //sideDist += vec3(mask) * deltaDist;
            //mapPos += ivec3(mask) * rayStep;
    //}

    // Ah this is for coloring obviously, seems to be odd though, no indexing
    //vec4 color;
    //if (mask.x) {
    //       color = vec4(0.5);
    //}
    //if (mask.y) {
    //       color = vec4(1.0);
    //}
    //if (mask.z) {
    //        color = vec4(0.75);
    //}
    //write_imagef(image, pixel, color);


}
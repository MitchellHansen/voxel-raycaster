
# About

This project is an implementation of an experimental "from scratch" volumetric rendering engine. Using OpenCL this program is able to hardware accelerate Woo's Fast Voxel Traversal Algorithm which traverses through a Sparse Voxel Octree as described by Laine et al. Light is simulated using the Blinn-Phong shading model along with a recursive ray tracer style of shadow tracing. Individual voxels are also textured using a user provided texture atlas and voxel data.

# Build

In order to build this project you must have Cmake, GLEW, SFML, and the OpenCL development libraries installed / downloaded. You're also going to need OpenCL compatable hardware that supports the cl_khr_gl_sharing extension. This project has, at some point, run on all three platforms: Windows (VS2017), Linux, and macOS. With a little bit of elbow grease it should still be able to compile and run on all these platforms. 

**Due to the deprecation of the OpenCL and OpenGL API's on macOS 10.14+ I will be removing mac support**

* Note on GLEW: macOS takes care of most of the "extension wrangling" so GLEW is not required when compiling for macOS

* Note on cl_khr_gl_sharing: The cl_khr_gl_sharing extension, or cl_APPLE_gl_sharing for macOS, is only supported on certain hardware. Further, cl_khr_gl_sharing is not supported on intel CPU's running linux. You might be able to install and use beignet to get around this. The program will warn you if your CL device doesn't support this extension.

# Screenshots

Video demo:

[![Video demo](http://img.youtube.com/vi/LqmXZJr_gxU/0.jpg)](http://www.youtube.com/watch?v=LqmXZJr_gxU)

Screenshots:

![alt tag](https://github.com/MitchellHansen/voxel-raycaster/blob/master/assets/screenshot1.png)
![alt tag](https://github.com/MitchellHansen/voxel-raycaster/blob/master/assets/screenshot.PNG)

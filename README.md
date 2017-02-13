
In order to build this project you must have Cmake, GLEW, SFML, and the OpenCL development libraries installed / downloaded. You're also going to need OpenCL compatable hardware that supports the cl_khr_gl_sharing extension. The VS2015 build is working, Linux compiles but I don't have hardware that works, and Mac should compile and run, but I don't have access to a Mac.

Note on cl_khr_gl_sharing: The cl_khr_gl_sharing extension, or cl_APPLE_gl_sharing for macOS, is only supported on certain hardware. Further, cl_khr_gl_sharing is not supported on intel CPU's running linux. You might be able to install and use beignet to get around this. The program will warn you if your CL device doesn't support this extension.

Featuring...

* Efficient Sparse Voxel Octrees, Laine et al.
* A fast voxel traversal algorithm for ray tracing, Woo et al.
* Blinn-Phong lighting
* Shadowing
* Textures

![alt tag](https://www.youtube.com/watch?v=DHcg2ZKend0)
![alt tag](https://github.com/MitchellHansen/voxel-raycaster/blob/master/assets/screenshot1.png)
![alt tag](https://github.com/MitchellHansen/voxel-raycaster/blob/master/assets/screenshot.PNG)

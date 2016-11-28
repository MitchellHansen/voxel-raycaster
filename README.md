Building this project is not difficult, but the setup prior can be. You must have OpenCL and SFML development libraries installed. You must also have OpenCL compatible hardware to run it on. The CMake file should take care of most of the configuration, but you have to point it to your SFML root directory before it will generate. Working in VS2015, I don't have Linux compatible hardware, but it does compile and run on the backup renderer albeit horrendously slowly. The macOS build was working around commit 50, but I no longer have access to a mac.

Papers used:

Efficient Sparse Voxel Octrees, Laine et al.

A fast voxel traversal algorithm for ray tracing, Woo et al.

![alt tag](https://github.com/MitchellHansen/voxel-raycaster/blob/master/assets/video.gif)

![alt tag](https://github.com/MitchellHansen/voxel-raycaster/blob/master/assets/screenshot.PNG)

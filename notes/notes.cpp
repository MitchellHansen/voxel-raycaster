// TODO

/*

OpenCL:
	- Add phong lighting / fix the current implementation
	- Switch to switch lighting models
	- Separate out into a part of the rendering module

Map:
	- Implement the new octree structure
		- storing the pre-octree volumetric data
		- determining when to load volumetric data into the in-memory structure
		- building the octree from that raw volumetric data
		- combining with other octree nodes to allow streaming of leafs
		- passing that data into the renderer
			- renderer needs to then traverse the octree
	- Terrain generation for real this time
	- Loader of 3rd party voxel data

Renderer:
	- Determine when to switch between the cpu and gpu rendering 
	- call to the map to make sure that the gpu/cpu has an up to date copy
	  of the volumetric data


	  
	  
	  
	  
	  
	  
	  
	  
Build:
	Z:\Cpp_Libs\SFML-Visual_Studio2015RCx64
	Z:/Cpp_Libs/glew-2.0.0/lib/Release/x64/glew32s.lib
	Z:/Cpp_Libs/glew-2.0.0/include
	  
*/


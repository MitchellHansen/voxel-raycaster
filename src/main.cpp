
/**
 *
 * Title:     Voxel Raycaster
 * Author:    Mitchell Hansen
 * License:   Apache 2.0
 * 
 * A little help from:
 *		John Amanatides & Andrew Woo: A Fast Voxel Traversal Algorithm forRay Tracing
 *		Samuli Laine & Tero Karras: Efficient Sparse Voxel Octrees
 *		
 * This project should not be viewed as a product or really anything other than
 * a cool little example on how real time volumetric ray marching is becoming 
 * very possible on todays hardware. Don't expect this program to work on your hardware,
 * perform as expected, have documentation, walk your dog, etc. 
 * 
 */

/**
 * TODO:
 * 
 * - Inconsistent lighting constants. GUI manipulation
 *      + Ancillary settings buffer and memory controller
 * - Attachment lookup and aux buffer, contour lookup & masking
 * - Traversal algorithm + related stacks and data structures
 *		+ Syncing, loading, unloading
 * - Octree, Map interface with the GPU
 * - Map/Oct raycasting algo that has parity with GPU
 * - Separate Application stages into areas that make sense
 * - Saving, loading of RLE voxel data and raw oct data
 * - Bodgy mem sharing of lights with the GPU
 * - Multi-stage rendering kernels
 * - Multi light support via first-strike resetting
 * - 
 *   
 */

#include "Application.h"

int main() {

	Application application;
	application.init_clcaster();
	application.init_events();
	application.game_loop();

	return 0;
}

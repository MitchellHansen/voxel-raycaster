#include "Renderer.h"

Renderer::Renderer() {

	cl = new CL_Wrapper();
	if (!cl->was_init_valid()) {
		delete cl;
		rc = new RayCaster();
	}
}

void Renderer::register_camera(Camera *camera) {
	this->camera = camera;
}

void Renderer::draw() {

}



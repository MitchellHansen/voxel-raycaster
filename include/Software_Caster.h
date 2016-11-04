#include "RayCaster.h"

class Software_Caster : public RayCaster
{
public:
	Software_Caster();

	virtual ~Software_Caster();

	int init() override;

	// In interop mode, this will create a GL texture that we share
	// Otherwise, it will create the pixel buffer and pass that in as an image, retrieving it each draw
	// Both will create the view matrix, view res buffer
	void create_viewport(int width, int height, float v_fov, float h_fov) override;

	void assign_lights(std::vector<Light> lights) override;
	void assign_map(Old_Map *map) override;
	void assign_camera(Camera *camera) override;
	void validate() override;

	// draw will abstract the gl sharing and software rendering
	// methods of retrieving the screen buffer
	void compute() override;
	void draw(sf::RenderWindow* window) override;

private:
	
	void cast_rays();

};

#pragma once
#include "Pub_Sub.h"

class FrameWatcher : public VrEventPublisher{
	


	
public:
	FrameWatcher();
	~FrameWatcher();

	void do_tick();

private:

	float get_elapsed_time();

	float step_size = 0.0166f;
	double frame_time = 0.0;
	double elapsed_time = 0.0;
	double delta_time = 0.0;
	double accumulator_time = 0.0;
	double current_time = 0.0;


};

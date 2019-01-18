#include "../include-legacy/FrameWatcher.h"


FrameWatcher::FrameWatcher() {

}

FrameWatcher::~FrameWatcher()
{

}

void FrameWatcher::do_tick() {


	elapsed_time = get_elapsed_time();
	delta_time = elapsed_time - current_time;
	current_time = elapsed_time;

	if (delta_time > 0.2f)
		delta_time = 0.2f;

	accumulator_time += delta_time;

	while ((accumulator_time - step_size) >= step_size) {
		accumulator_time -= step_size;

		// ==== DELTA TIME LOCKED ====
	}

}

float FrameWatcher::get_elapsed_time() {

	static std::chrono::time_point<std::chrono::system_clock> start;
	static bool started = false;

	if (!started) {
		start = std::chrono::system_clock::now();
		started = true;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = now - start;
	return static_cast<float>(elapsed_time.count());
}


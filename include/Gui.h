#pragma once
#include <mutex>
#include <Logger.h>
#include <list>

/**
 *
 * GUI
 *
 * Any class that wants to have an interactive GUI rendered to the window may
 * inherit GUI and override the render_gui() and update_gui() methods
 *
 * ImGui operations must be completely wrapped in Begins and Ends
 *
 * You may enable and disable rendering by setting the 'rendering' flag to true or false
 *
 */

class Gui {
	
public:

	Gui() {
		container_lock.lock();
		renderable_container.push_back(this);
		container_lock.unlock();
	};
	virtual ~Gui() {
		container_lock.lock();
		renderable_container.remove(this);	
		container_lock.unlock();
	};

	virtual void render_gui() = 0;
	virtual void update_gui() = 0;

	// Instead of rendering nil, we can pass our render call if we would like
	bool renderable() { return rendering; };
	
private:

	// Whatever class that wants to call this must be a friend!!!
	friend class Application;
	static void do_render() {
		for (auto i : renderable_container) {
			i->update_gui();
			if (i->renderable())
				i->render_gui();
		}
	};

	

	static std::mutex container_lock;
	static std::list<Gui*> renderable_container;

protected:
	bool rendering = false;
	// Derived class will handle imgui calls
	
};



















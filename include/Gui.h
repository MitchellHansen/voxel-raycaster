#pragma once
#include <mutex>
#include <Logger.h>
#include <list>

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
	bool rendering = true;
	// Derived class will handle imgui calls
	
};



















#include "Gui.h"

std::mutex Gui::container_lock;
std::list<Gui*> Gui::renderable_container;
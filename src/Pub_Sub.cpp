#include <vector>
#include "Event.hpp"
#include "Pub_Sub.h"


void VrEventSubscriber::subscribe_to_publisher(VrEventPublisher* publisher, vr::Event::EventType type) {

	publisher->subscribe(this, type);
}

void VrEventSubscriber::subscribe_to_publisher(VrEventPublisher* publisher, std::vector<vr::Event::EventType> type) {

	publisher->subscribe(this, type);
}

void VrEventPublisher::subscribe(VrEventSubscriber *subscriber, vr::Event::EventType type) {
	
	subscribers[type].push_back(subscriber);
}

void VrEventPublisher::subscribe(VrEventSubscriber *subscriber, std::vector<vr::Event::EventType> type) {
	
	for (auto t : type)
		subscribers[t].push_back(subscriber);
}

void VrEventPublisher::unsubscribe(VrEventSubscriber *s, vr::Event::EventType type) {
	
	std::remove(subscribers[type].begin(), subscribers[type].end(), s);
}

void VrEventPublisher::notify_subscribers(std::unique_ptr<vr::Event> event) {

	// get the bucket containing subscribers to that Event_Class
	std::vector<VrEventSubscriber*> *event_type_bucket = &subscribers[event.get()->type];

	// Send them the event
	for (auto s : *event_type_bucket) {
		s->recieve_event(this, std::move(event));
	}
}

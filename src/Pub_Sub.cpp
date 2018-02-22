#include "Pub_Sub.h"


/**
 * Subscriber
 */
VrEventSubscriber::~VrEventSubscriber() {

	// Cycles through the publishers we're subscribed to
	for (auto const& publisher : subscriptions) {

		// And one by one remove the EventTypes we're subscribed to
		for (auto event_type: publisher.second) {
            publisher.first->unsubscribe(this, event_type);
        }
	}
}

void VrEventSubscriber::subscribe_to_publisher(VrEventPublisher* publisher, vr::Event::EventType type) {

	publisher->subscribe(this, type);

	subscriptions[publisher].insert(type);
}

void VrEventSubscriber::subscribe_to_publisher(VrEventPublisher* publisher, std::vector<vr::Event::EventType> type) {

	publisher->subscribe(this, type);

	subscriptions[publisher].insert(type.begin(), type.end());
}

void VrEventSubscriber::unsubscribe(VrEventPublisher* publisher, vr::Event::EventType type){

    if (subscriptions.count(publisher)){
        std::set<vr::Event::EventType> set = subscriptions[publisher];
        auto it = set.find (type);
        set.erase (it, set.end());
    }
}

void VrEventSubscriber::unsubscribe_all(VrEventPublisher* publisher){

    if (subscriptions.count(publisher)){
        subscriptions.erase(publisher);
    }
}

/**
 * Publisher
 */
VrEventPublisher::~VrEventPublisher() {

	// Cycle through the subscribers that are listening to us
	for (auto const& event_bucket : subscribers) {

		// And one by one remove the subscriber
		for (auto subscriber: event_bucket.second){
			subscriber->unsubscribe(this, event_bucket.first);
		}
	}
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
	//std::vector<VrEventSubscriber*> *event_type_bucket = &subscribers[event->type];

	// Send them the event
	// Each and every event that is received in the event_handler function
	// will be a unique ptr solely owned by that function
	for (auto s : subscribers[event->type]) {
        s->event_handler(this, event->clone());
	}

}


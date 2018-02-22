#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <SFML/Graphics.hpp>
#include "Event.hpp"

class VrEventPublisher;



/**
 * VrEventSubscriber
 */
class VrEventSubscriber {
public:
	virtual ~VrEventSubscriber();

    // Recieve an event from a publisher, event must be cast to it's respective event type
	virtual void event_handler(VrEventPublisher *publisher, std::unique_ptr<vr::Event> event) = 0;

    // Subscribes to the publisher, keeps track of the ptr and the relevent event types
	void subscribe_to_publisher(VrEventPublisher* publisher, vr::Event::EventType type);
	void subscribe_to_publisher(VrEventPublisher* publisher, std::vector<vr::Event::EventType> type);

    // Looks for the publisher ptr and event type in the subscriptions map. If there, Removes them
    void unsubscribe(VrEventPublisher* publisher, vr::Event::EventType type);
    void unsubscribe_all(VrEventPublisher* publisher);

protected:

    // When we destroy a subscriber we need to be able to notify the publishers
    // We have to keep track of every EventType because of the way EventTypes
    // are mapped to subscribers in the publisher
	std::map<VrEventPublisher*, std::set<vr::Event::EventType>> subscriptions;
};


class VrEventPublisher {
public:

	virtual ~VrEventPublisher();

    // Adds the subscriber ptr to the bucket[event_type]
	virtual void subscribe(VrEventSubscriber *subscriber, vr::Event::EventType type);
	virtual void subscribe(VrEventSubscriber *subscriber, std::vector<vr::Event::EventType> type);

    // Removes the subscriber ptr from the specified bucket[type]
    // If subscribed to multiple events, unsubscribe must be called for each event
    virtual void unsubscribe(VrEventSubscriber *s, vr::Event::EventType c);

    // Trigger the publisher to notify it's subscribers to the specified event
	virtual void notify_subscribers(std::unique_ptr<vr::Event> event);

private:

	std::map<vr::Event::EventType, std::vector<VrEventSubscriber*>> subscribers;

};


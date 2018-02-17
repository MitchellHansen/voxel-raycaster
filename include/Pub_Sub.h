#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Event.hpp"
#include <memory>


class VrEventPublisher;

class VrEventSubscriber {
public:
	virtual ~VrEventSubscriber();
	virtual void recieve_event(VrEventPublisher* publisher, std::unique_ptr<vr::Event> event) = 0;
	void subscribe_to_publisher(VrEventPublisher* publisher, vr::Event::EventType type);
	void subscribe_to_publisher(VrEventPublisher* publisher, std::vector<vr::Event::EventType> type);
	void unsubscribe(VrEventPublisher* publisher, vr::Event::EventType type);
protected:

    // When we destroy a subscriber we need to be able to notify the publishers
    // We have to keep track of every EventType because of the way EventTypes
    // are mapped to subscribers in the publisher
	std::map<VrEventPublisher*, std::vector<vr::Event::EventType>> subscriptions;
};


class VrEventPublisher {
public:

	virtual ~VrEventPublisher();
	virtual void subscribe(VrEventSubscriber *subscriber, vr::Event::EventType type);
	virtual void subscribe(VrEventSubscriber *subscriber, std::vector<vr::Event::EventType> type);
	virtual void unsubscribe(VrEventSubscriber *s, vr::Event::EventType c);
	virtual void notify_subscribers(std::unique_ptr<vr::Event> event);
private:
	std::map<vr::Event::EventType, std::vector<VrEventSubscriber*>> subscribers;

};


#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Event.hpp"
#include <memory>


class VrEventPublisher;

class VrEventSubscriber {
public:
	virtual ~VrEventSubscriber() {};
	virtual void recieve_event(VrEventPublisher* publisher, std::unique_ptr<vr::Event> event) = 0;
	void subscribe_to_publisher(VrEventPublisher* publisher, vr::Event::EventType type);
	void subscribe_to_publisher(VrEventPublisher* publisher, std::vector<vr::Event::EventType> type);
protected:
	std::vector<vr::Event::EventType> subscribed_event_types;
};


class VrEventPublisher {
public:

	virtual ~VrEventPublisher() {};
	virtual void subscribe(VrEventSubscriber *subscriber, vr::Event::EventType type);
	virtual void subscribe(VrEventSubscriber *subscriber, std::vector<vr::Event::EventType> type);
	virtual void unsubscribe(VrEventSubscriber *s, vr::Event::EventType c);
	virtual void notify_subscribers(std::unique_ptr<vr::Event> event);
private:
	std::map<vr::Event::EventType, std::vector<VrEventSubscriber*>> subscribers;

};


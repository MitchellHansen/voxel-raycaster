#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Event.hpp"


class VrEventPublisher;

class VrEventSubscriber {
public:
	virtual ~VrEventSubscriber() {};
	virtual void update(VrEventPublisher* p, vr::Event e) = 0;
	void subscribe(VrEventPublisher* publisher, vr::Event::EventType type);
	void subscribe(VrEventPublisher* publisher, std::vector<vr::Event::EventType> type);
protected:
	std::vector<vr::Event::EventType> subscribed_event_types;
};


class VrEventPublisher {
public:

	virtual ~VrEventPublisher() {};
	virtual void subscribe(VrEventSubscriber *subscriber, vr::Event::EventType type);
	virtual void subscribe(VrEventSubscriber *subscriber, std::vector<vr::Event::EventType> type);
	virtual void unsubscribe(VrEventSubscriber *s, vr::Event::EventType c);
	virtual void notify(vr::Event e);
private:
	std::map<vr::Event::EventType, std::vector<VrEventSubscriber*>> subscribers;

};


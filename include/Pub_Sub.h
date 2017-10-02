#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Event.hpp"
#include <memory>
#include <list>


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
	virtual void subscribe(VrEventSubscriber *subscriber, vr::Event::EventType type) final;
	virtual void subscribe(VrEventSubscriber *subscriber, std::vector<vr::Event::EventType> type) final;
	virtual void unsubscribe(VrEventSubscriber *s, vr::Event::EventType c) final;
	
private:
	
	std::map<vr::Event::EventType, std::vector<VrEventSubscriber*>> subscribers;
	
protected:
	virtual void notify_subscribers(std::unique_ptr<vr::Event> event) final;
	virtual void dispatch_events() final;
	virtual void generate_events() = 0; 
	std::list<std::unique_ptr<vr::Event>> event_queue;

};


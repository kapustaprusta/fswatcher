#pragma once

#include <list>

class Event;

namespace filesystem
{

class IEventSub
{
public:
	IEventSub() = default;
	virtual ~IEventSub() = default;

	virtual void AddEvent(const Event &event) = 0;
	virtual void AddEvents(const std::list<Event> &events) = 0;
};

} // namespace filesystem
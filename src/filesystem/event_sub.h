#pragma once

#include "event.h"

namespace filesystem
{

class IEventSub
{
public:
	IEventSub() = default;
	virtual ~IEventSub() = default;

	virtual void AddEvent(const Event &event) = 0;
};

} // namespace filesystem
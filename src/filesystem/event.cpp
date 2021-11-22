#include "event.h"

namespace filesystem
{

Event::Event()
{

}

Event::Event(Event &&event)
{
	isDir_ = std::move(event.isDir_);
	type_  = std::move(event.type_ );
	name_  = std::move(event.name_ );
}

Event::Event(const std::string &name,
			 bool isDir,
			 const eType &type)
	: isDir_(isDir)
	, type_(type)
	, name_(name)
{

}

Event::~Event()
{

}

void Event::SetType(const eType &type)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	type_ = type;
}

Event::eType Event::Type() const
{
	return type_;
}

void Event::SetIsDir(bool isDir)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	isDir_ = isDir;
}

bool Event::IsDir() const
{
	return isDir_;
}

void Event::SetName(const std::string &name)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	name_ = name;
}

std::string Event::Name() const
{
	return name_;
}

} // namespace filesystem
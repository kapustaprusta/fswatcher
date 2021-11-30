#include <iostream>
#include <unistd.h>
#include <exception>

#include "watcher.h"
#include "event_sub.h"

namespace filesystem
{

Watcher::Watcher()
	: isRunning_(false)
	, eventSize_(sizeof(inotify_event))
	, eventsBufferSize_(1024*(eventSize_+16))
{
	notifyDescr_ = inotify_init();
	if (notifyDescr_ < 0)
	{
		throw std::runtime_error("Cannot initialize watcher");
	}
}

Watcher::~Watcher()
{
	Stop();
	CloseDescr();
}

void Watcher::Start()
{
	if (isRunning_.load())
	{
		return;
	}

	workingThread_
			= std::thread(&Watcher::WaitForEvents, this);
	isRunning_.store(true);
}

void Watcher::Stop()
{
	if (!isRunning_.load())
	{
		return;
	}

	isRunning_.store(false);
	if (workingThread_.joinable())
	{
		workingThread_.join();
	}
}

bool Watcher::AddNode(const std::string &nodePath)
{
	const uint32_t watchDescr = inotify_add_watch(notifyDescr_,
												  nodePath.c_str(),
												  IN_MODIFY | IN_CREATE | IN_DELETE);

	if (watchDescr < 0)
	{
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(membersMutex_);

		watchDescrs_.insert(std::make_pair(nodePath,
										   watchDescr));
	}

	return true;
}

bool Watcher::RemoveNode(const std::string &nodePath)
{
	std::lock_guard<std::mutex> lock(membersMutex_);
	
	std::map<std::string, uint32_t>::iterator itDeletedNode
												= watchDescrs_.find(nodePath);
	if (itDeletedNode == watchDescrs_.end())
	{
		return true;
	}

	const uint32_t watchDescr = inotify_rm_watch(notifyDescr_,
												 itDeletedNode->second);
	if (watchDescr < 0)
	{
		return false;
	}

	watchDescrs_.erase(nodePath);

	return true;
}

void Watcher::AddEventSub(std::shared_ptr<IEventSub> &sub)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	lEventsSubs_.push_back(sub);
}

void Watcher::RemoveEventSub(const std::shared_ptr<IEventSub> &sub)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	lEventsSubs_.remove(sub);
}

bool Watcher::CloseDescr()
{
	const int retCode = close(notifyDescr_);

	return retCode == 0;
}

void Watcher::WaitForEvents()
{
	fd_set descriptors;
	timeval time_to_wait;

	time_to_wait.tv_sec = 3;
	time_to_wait.tv_usec = 0;

	uint64_t eventsIdx = 0;
	std::list<Event> lEvents;
	eventsBuffer_.resize(eventsBufferSize_);

	while(isRunning_.load())
	{
		eventsIdx = 0;
		lEvents.clear();
		eventsBuffer_.clear();

		FD_ZERO(              &descriptors);
		FD_SET (notifyDescr_, &descriptors);

		int readyDescriptorsNumber = select(FD_SETSIZE,
											&descriptors,
											NULL,
											NULL,
											&time_to_wait);
		
		if (readyDescriptorsNumber <= 0)
		{
			continue;
		}

		ssize_t eventsLength = read(notifyDescr_,
									eventsBuffer_.data(),
									eventsBufferSize_);

		if (eventsLength <= 0)
		{
			continue;
		}

		while (eventsIdx < eventsLength)
		{
			inotify_event *iEvent
						= (inotify_event *) &eventsBuffer_[eventsIdx];
			if (iEvent->len != 0)
			{
				Event event(iEvent->name,
							iEvent->mask & IN_ISDIR);

				if (iEvent->mask & IN_MODIFY)
				{
					event.SetType(Event::WAS_MODIFIED);
				}
				else if (iEvent->mask & IN_CREATE) 
				{
					event.SetType(Event::WAS_CREATED);
				}
				else if (iEvent->mask & IN_DELETE) 
				{
					event.SetType(Event::WAS_DELETED);
				}

				lEvents.push_back(std::move(event));

				eventsIdx += eventSize_ + iEvent->len;
			}
		}

		NotifySubs(std::move(lEvents));
	}
}

void Watcher::NotifySubs(std::list<Event> events)
{
	for (const auto &crEventsSub : lEventsSubs_)
	{
		crEventsSub->AddEvents(events);
	}
}

}/* namespace filesystem */

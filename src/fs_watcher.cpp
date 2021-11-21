#include "fs_watcher.h"

#include <iostream>
#include <unistd.h>
#include <exception>

namespace file_system
{

FSWatcherEvent::FSWatcherEvent()
{

}

FSWatcherEvent::FSWatcherEvent(FSWatcherEvent &&FSEvent)
{
	isDir_ = std::move(FSEvent.isDir_);
	type_  = std::move(FSEvent.type_ );
	name_  = std::move(FSEvent.name_ );
}

FSWatcherEvent::FSWatcherEvent(const std::string &name,
							   bool isDir,
							   const eType &type)
	: isDir_(isDir)
	, type_(type)
	, name_(name)
{

}

FSWatcherEvent::~FSWatcherEvent()
{

}

void FSWatcherEvent::SetType(const eType &type)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	type_ = type;
}

FSWatcherEvent::eType FSWatcherEvent::Type() const
{
	return type_;
}

void FSWatcherEvent::SetIsDir(bool isDir)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	isDir_ = isDir;
}

bool FSWatcherEvent::IsDir() const
{
	return isDir_;
}

void FSWatcherEvent::SetName(const std::string &name)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	name_ = name;
}

std::string FSWatcherEvent::Name() const
{
	return name_;
}

FSWatcher::FSWatcher()
	: isStop_(false)
	, eventSize_(sizeof(inotify_event))
	, eventsBufferSize_(1024*(eventSize_+16))
{
	notifyDescr_ = inotify_init();
	if (notifyDescr_ < 0)
	{
		throw std::runtime_error("Cannot initialize watcher");
	}
}

FSWatcher::~FSWatcher()
{
	Stop();
	CloseDescr();
}

void FSWatcher::Start()
{
	workingThread_
			= std::thread(&FSWatcher::WaitForEvents, this);
}

void FSWatcher::Stop()
{
	isStop_.store(true);
	if (workingThread_.joinable())
	{
		workingThread_.join();
	}
}

bool FSWatcher::AddNode(const std::string &crstrNode)
{
	const uint32_t watchDescr = inotify_add_watch(notifyDescr_,
												  crstrNode.c_str(),
												  IN_MODIFY | IN_CREATE | IN_DELETE);

	if (watchDescr < 0)
	{
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(membersMutex_);

		watchDescrs_.insert(std::make_pair(crstrNode,
										   watchDescr));
	}

	return true;
}

bool FSWatcher::RemoveNode(const std::string &crstrNode)
{
	std::lock_guard<std::mutex> lock(membersMutex_);
	
	std::map<std::string, uint32_t>::iterator itDeletedNode
												= watchDescrs_.find(crstrNode);
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

	watchDescrs_.erase(crstrNode);

	return true;
}

void FSWatcher::AddEventSub(std::shared_ptr<IFSWatcherEventSub> &rEventSub)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	lEventsSubs_.push_back(rEventSub);
}

void FSWatcher::RemoveEventSub(const std::shared_ptr<IFSWatcherEventSub> &crEventSub)
{
	std::lock_guard<std::mutex> lock(membersMutex_);

	lEventsSubs_.remove(crEventSub);
}

bool FSWatcher::CloseDescr()
{
	const int retCode = close(notifyDescr_);

	return retCode == 0;
}

void FSWatcher::WaitForEvents()
{
	fd_set descriptors;
	timeval time_to_wait;

	time_to_wait.tv_sec = 3;
	time_to_wait.tv_usec = 0;

	uint64_t eventsIdx = 0;
	eventsBuffer_.resize(eventsBufferSize_);

	while(!isStop_.load())
	{
		eventsIdx = 0;
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

		printf("EVENTS: %ld\n", eventsLength);

		if (eventsLength <= 0)
		{
			continue;
		}

		while (eventsIdx < eventsLength)
		{
			inotify_event *event
						= (inotify_event *) &eventsBuffer_[eventsIdx];
			if (event->len != 0)
			{
				FSWatcherEvent FSEvent(event->name,
									   event->mask & IN_ISDIR);

				if (event->mask & IN_MODIFY)
				{
					FSEvent.SetType(FSWatcherEvent::WAS_MODIFIED);
				}
				else if (event->mask & IN_CREATE) 
				{
					FSEvent.SetType(FSWatcherEvent::WAS_CREATED);
				}
				else if (event->mask & IN_DELETE) 
				{
					FSEvent.SetType(FSWatcherEvent::WAS_DELETED);
				}

				NotifySubs(std::move(FSEvent));

				eventsIdx += eventSize_ + event->len;
			}
		}
	}
}

void FSWatcher::NotifySubs(FSWatcherEvent FSEvent)
{
	for (const auto &sub : lEventsSubs_)
	{
		sub->AddEvent(FSEvent);
	}
}

}/* namespace file_system */

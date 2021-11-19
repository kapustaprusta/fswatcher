#include "fs_watcher.h"

#include <iostream>
#include <unistd.h>
#include <exception>

namespace file_system
{

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

	watchDescrs_.insert(std::make_pair(crstrNode,
									   watchDescr));

	return true;
}

bool FSWatcher::RemoveNode(const std::string &crstrNode)
{
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

void FSWatcher::AddEventSub(IFSWatcherEventSub *pEventSub)
{
	lEventsSubs_.push_back(pEventSub);
}

void FSWatcher::RemoveEventSub(IFSWatcherEventSub *pEventSub)
{
	lEventsSubs_.remove(pEventSub);
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
				if (event->mask & IN_MODIFY) 
				{
					if (event->mask & IN_ISDIR)
					{
						printf("The directory %s was modified.\n", event->name);
					}
					else
					{
						printf("The file %s was modified.\n", event->name);
					}
				}

				if (event->mask & IN_CREATE) 
				{
					if (event->mask & IN_ISDIR)
					{
						printf("The directory %s was created.\n", event->name);
					}
					else
					{
						printf("The file %s was created.\n", event->name);
					}
				}

				if (event->mask & IN_DELETE) 
				{
					if (event->mask & IN_ISDIR)
					{
						printf("The directory %s was deleted.\n", event->name);
					}
					else
					{
						printf("The file %s was deleted.\n", event->name);
					}
				}

				eventsIdx += eventSize_ + event->len;
			}
		}
	}
}

}/* namespace file_system */

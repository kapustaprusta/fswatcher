#pragma once

#include <map>
#include <list>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <string>
#include <vector>
#include <sys/inotify.h>

#include "event.h"

namespace filesystem
{

class IEventSub;

class Watcher
{
public:
	Watcher();
	~Watcher();

	void Start();
	void Stop();

	bool AddNode(const std::string &nodePath);
	bool RemoveNode(const std::string &nodePath);

	void AddEventSub(std::shared_ptr<IEventSub> &sub);
	void RemoveEventSub(const std::shared_ptr<IEventSub> &sub);

private:
	bool CloseDescr();
	void WaitForEvents();

	void NotifySubs(std::list<Event> events);

	/** */
	std::atomic_bool isRunning_;

	/** */
	uint32_t notifyDescr_;

	/** */
	const uint32_t eventSize_;

	/** */
	const uint32_t eventsBufferSize_;

	/** */
	std::mutex membersMutex_;

	/** */
	std::thread workingThread_;

	/** */
	std::vector<uint8_t> eventsBuffer_;

	/** */
	std::map<std::string, uint32_t> watchDescrs_;

	/** */
	std::list<std::shared_ptr<IEventSub>> lEventsSubs_;
};

} // namespace filesystem
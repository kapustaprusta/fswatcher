#include <map>
#include <list>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <cstdint>
#include <sys/inotify.h>

namespace file_system
{

class IFSWatcherEventSub
{
public:
	IFSWatcherEventSub() = default;
	virtual ~IFSWatcherEventSub() = default;

	virtual void AddEvent() = 0;
};

class FSWatcher
{
public:
	FSWatcher();
	~FSWatcher();

	void Start();
	void Stop();

	bool AddNode(const std::string &crstrNode);
	bool RemoveNode(const std::string &crstrNode);

	void AddEventSub(IFSWatcherEventSub *pEventSub);
	void RemoveEventSub(IFSWatcherEventSub *pEventSub);

private:
	bool CloseDescr();
	void WaitForEvents();

	/** */
	uint32_t notifyDescr_;

	/** */
	const uint32_t eventSize_;

	/** */
	const uint32_t eventsBufferSize_;

	/** */
	std::thread workingThread_;

	/** */
	std::atomic_bool isStop_;

	/** */
	std::vector<uint8_t> eventsBuffer_;

	/** */
	std::list<IFSWatcherEventSub*> lEventsSubs_;

	/** */
	std::map<std::string, uint32_t> watchDescrs_;
};

}
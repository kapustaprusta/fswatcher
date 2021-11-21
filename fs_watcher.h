#include <map>
#include <list>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <cstdint>
#include <sys/inotify.h>

namespace file_system
{

class FSWatcherEvent
{
public:
	enum eType
	{
		UNKNOWN      = 0,
		WAS_CREATED  = 1,
		WAS_DELETED  = 2,
		WAS_MODIFIED = 3,
	};

	FSWatcherEvent();
	FSWatcherEvent(const std::string &name,
				   const eType &type,
				   bool isDir = false);
	~FSWatcherEvent();

	void SetType(const eType &type);
	eType Type() const;

	void SetIsDir(bool isDir);
	bool IsDir() const;

	void SetName(const std::string &name);
	std::string Name() const;

private:
	/** */
	bool isDir_;

	/** */
	eType type_;

	/** */
	std::string name_;

	/** */
	std::mutex membersMutex_;
};

class IFSWatcherEventSub
{
public:
	IFSWatcherEventSub() = default;
	virtual ~IFSWatcherEventSub() = default;

	virtual void AddEvent(const FSWatcherEvent &crEvent) = 0;
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

	void AddEventSub(std::shared_ptr<IFSWatcherEventSub> &rEventSub);
	void RemoveEventSub(const std::shared_ptr<IFSWatcherEventSub> &crEventSub);

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
	std::atomic_bool isStop_;

	/** */
	std::mutex membersMutex_;

	/** */
	std::thread workingThread_;

	/** */
	std::vector<uint8_t> eventsBuffer_;

	/** */
	std::map<std::string, uint32_t> watchDescrs_;

	/** */
	std::list<std::shared_ptr<IFSWatcherEventSub>> lEventsSubs_;
};

}
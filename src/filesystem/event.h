#include <mutex>
#pragma once

#include <string>

namespace filesystem
{

class Event
{
public:
	enum eType
	{
		UNKNOWN      = 0,
		WAS_CREATED  = 1,
		WAS_DELETED  = 2,
		WAS_MODIFIED = 3,
	};

	Event();
	Event(Event &&FSEvent);
	Event(const std::string &name,
		  bool isDir = false,
		  const eType &type = UNKNOWN);
	~Event();

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

} // namespace filesystem
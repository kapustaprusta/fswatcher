#include <memory>
#include <iostream>

#include "src/filesystem/watcher.h"
#include "src/filesystem/event_sub.h"

class MySub : public filesystem::IEventSub
{
public:
	MySub() = default;
	~MySub() = default;

	void AddEvent(const filesystem::Event &event) override
	{
		std::cout << "EVENT NAME: " << event.Name() << "\n";
	}

	void AddEvents(const std::list<filesystem::Event> &events) override
	{
		for (const auto &event : events)
		{
			std::cout << "EVENT NAME: " << event.Name() << "\n";
		}
	}
};

int main()
{
	filesystem::Watcher watcher;
	watcher.AddNode("/home/vniksihov/Test");
	watcher.Start();

	std::shared_ptr<filesystem::IEventSub> pEventSub
											= std::make_shared<MySub>();
	watcher.AddEventSub(pEventSub);

	int i = 0;
	std::cin >> i;

	return 0;
}
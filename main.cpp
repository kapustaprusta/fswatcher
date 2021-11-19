#include <iostream>

#include "fs_watcher.h"

int main()
{
	file_system::FSWatcher watcher;
	watcher.AddNode("/home/vniksihov/Test");
	watcher.Start();

	int i = 0;
	std::cin >> i;

	return 0;
}
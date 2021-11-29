#include <memory>
#include <iostream>

#include "src/filesystem/watcher.h"

int main()
{
	filesystem::Watcher watcher;
	watcher.AddNode("/home/vniksihov/Test");
	watcher.Start();

	int i = 0;
	std::cin >> i;

	return 0;
}
project(fs_watcher)

cmake_minimum_required(VERSION 3.21)

set(HEADERS fs_watcher.h)

set(SOURCES main.cpp
			fs_watcher.cpp)

add_executable(fs_watcher ${HEADERS} ${SOURCES})

target_link_libraries(fs_watcher pthread)
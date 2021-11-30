project(fswatcher)

cmake_minimum_required(VERSION 3.21)

set(HEADERS src/filesystem/event.h
            src/filesystem/watcher.h
            src/filesystem/event_sub.h)

set(SOURCES main.cpp
            src/filesystem/event.cpp
            src/filesystem/watcher.cpp)

add_executable(fswatcher ${HEADERS}
                        ${SOURCES})

target_link_libraries(fswatcher pthread)
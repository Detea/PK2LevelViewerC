cmake_minimum_required(VERSION 3.5)
project(PK2LevelViewer)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

add_executable(PK2LevelViewer
	LevelViewer.h
	LevelViewer.c

	PK2Map.h

	util.h
	util.c

	main.c)

target_link_libraries(PK2LevelViewer PRIVATE SDL2::SDL2)

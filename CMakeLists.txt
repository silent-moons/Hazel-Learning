cmake_minimum_required(VERSION 3.17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DHZ_DEBUG)
endif()

project(HazelEngine)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Sandbox)
add_subdirectory(Hazel)

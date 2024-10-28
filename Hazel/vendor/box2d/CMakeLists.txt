file(GLOB_RECURSE box2d "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(box2dlib STATIC
	${box2d}
)

target_include_directories(box2dlib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src
)
file(GLOB_RECURSE yamlcpp "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(yamlcpplib STATIC
	${yamlcpp}
)

target_include_directories(yamlcpplib PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

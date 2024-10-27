file(GLOB_RECURSE Hazel "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_subdirectory(vendor/imgui)
add_subdirectory(vendor/yaml-cpp)

add_definitions(-DGLFW_INCLUDE_NONE)

add_library(libHazel STATIC 
	${Hazel}
	${CMAKE_SOURCE_DIR}/Hazel/vendor/stb_image/stb_image.cpp
	${CMAKE_SOURCE_DIR}/Hazel/vendor/ImGuizmo/ImGuizmo.cpp
)

target_include_directories(libHazel PUBLIC
	${CMAKE_SOURCE_DIR}/ThirdParty/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/entt/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/spdlog/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/imgui
	${CMAKE_SOURCE_DIR}/Hazel/vendor/stb_image
	${CMAKE_SOURCE_DIR}/Hazel/vendor/yaml-cpp/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/ImGuizmo
	${CMAKE_SOURCE_DIR}/Hazel/vendor
	${CMAKE_SOURCE_DIR}/Hazel/src
)

target_link_directories(libHazel PUBLIC
	SYSTEM ${CMAKE_SOURCE_DIR}/ThirdParty/lib
)

# 设置预编译头文件
target_precompile_headers(libHazel PRIVATE 
	${CMAKE_SOURCE_DIR}/Hazel/src/hzpch.h
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)  
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)  
endif()

target_link_libraries(libHazel PRIVATE
	glfw3
	opengl32
	imguilib
	yamlcpplib
)
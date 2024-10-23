add_executable(HazelEngine 
	"src/SandboxApp.cpp" 
	"${CMAKE_SOURCE_DIR}/ThirdParty/src/glad.c"
)

target_include_directories(HazelEngine PRIVATE
	${CMAKE_SOURCE_DIR}/ThirdParty/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/spdlog/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/imgui
	${CMAKE_SOURCE_DIR}/Hazel/vendor
	${CMAKE_SOURCE_DIR}/Hazel/src
)

target_link_libraries(HazelEngine PRIVATE 
	libHazel
)
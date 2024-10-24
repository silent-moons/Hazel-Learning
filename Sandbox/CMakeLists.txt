file(GLOB ASSETS "./assets" )

file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR}/Sandbox)

add_executable(HazelEngine 
	"src/SandboxApp.cpp" 
	"src/Sandbox2D.cpp" 
	"src/ExampleLayer.cpp" 
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
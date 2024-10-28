file(GLOB ASSETS "./assets" )
file(GLOB RESOURCES "./Resources" )

file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR}/Hazel-Editor)
file(COPY ${RESOURCES} DESTINATION ${CMAKE_BINARY_DIR}/Hazel-Editor)

add_executable(HazelEditor 
	"src/EditorLayer.cpp" 
	"src/HazelEditorApp.cpp" 
	"src/Panels/SceneHierarchyPanel.cpp" 
	"src/Panels/ContentBrowserPanel.cpp" 
	"${CMAKE_SOURCE_DIR}/ThirdParty/src/glad.c"
)

target_include_directories(HazelEditor PRIVATE
	${CMAKE_SOURCE_DIR}/ThirdParty/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/spdlog/include
	${CMAKE_SOURCE_DIR}/Hazel/vendor/imgui
	${CMAKE_SOURCE_DIR}/Hazel/vendor
	${CMAKE_SOURCE_DIR}/Hazel/src
)

target_link_libraries(HazelEditor PRIVATE 
	libHazel
)
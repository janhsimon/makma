cmake_minimum_required(VERSION 3.1)
project(makma)

add_subdirectory(external)

include(FindVulkan)

file(GLOB_RECURSE SOURCE LIST_DIRECTORIES false "src/*.cpp" "src/*.hpp")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})
add_executable(${PROJECT_NAME} ${SOURCE})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} assimp glm imgui imguizmo SDL2 SDL2main stb ${Vulkan_LIBRARIES})

set(COPY_DIRECTORIES
  Models
  Shaders
  Textures
)

install(DIRECTORY ${COPY_DIRECTORIES} DESTINATION ${CMAKE_BINARY_DIR})
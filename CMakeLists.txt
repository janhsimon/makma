cmake_minimum_required(VERSION 3.1)
project(makma)

add_subdirectory(external)

include(FindVulkan)

file(GLOB_RECURSE SOURCE LIST_DIRECTORIES false "src/*.cpp" "src/*.hpp")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})
add_executable(makma ${SOURCE})

set_property(TARGET makma PROPERTY CXX_STANDARD 17)
target_include_directories(makma PRIVATE "external/assimp/include" "build/external/assimp/include" "external/glm" "external/imgui" "external/imguizmo" "external/sdl/include" "external/stb" "external/sdl/include" ${Vulkan_INCLUDE_DIRS})
target_link_libraries(makma assimp imgui imguizmo SDL2-static SDL2main ${Vulkan_LIBRARIES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT makma)
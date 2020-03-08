cmake_minimum_required(VERSION 3.1)
project(Makma)

file(GLOB_RECURSE SOURCE LIST_DIRECTORIES false "src/*.cpp" "src/*.hpp")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})
add_executable(Makma ${SOURCE})

set_property(TARGET Makma PROPERTY CXX_STANDARD 17)
#include_directories("../graphics/include" "../interface/include")
#target_link_libraries(editor graphics interface)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Makma)
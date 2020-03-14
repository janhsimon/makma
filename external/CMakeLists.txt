project(external)

# assimp
add_subdirectory(assimp)
set_target_properties(assimp PROPERTIES FOLDER "external")

#imgui
file(GLOB IMGUI_SOURCE LIST_DIRECTORIES false "imgui/*.cpp" "imgui/*.h")
add_library(imgui STATIC ${IMGUI_SOURCE})
set_target_properties(imgui PROPERTIES FOLDER "external")

#imguizmo
file(GLOB IMGUIZMO_SOURCE LIST_DIRECTORIES false "imguizmo/*.cpp" "imguizmo/*.h")
add_library(imguizmo STATIC ${IMGUIZMO_SOURCE} ${IMGUI_SOURCE})
target_include_directories(imguizmo PRIVATE "imgui")
set_target_properties(imguizmo PROPERTIES FOLDER "external")

# SDL
add_subdirectory(SDL)
set_target_properties(SDL2 PROPERTIES FOLDER "external")
set_target_properties(SDL2main PROPERTIES FOLDER "external")
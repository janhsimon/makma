set(SOURCE_CORE
  core/Camera.cpp
  core/Camera.hpp

  core/Game.cpp
  core/Game.hpp

  core/Input.cpp
  core/Input.hpp

  core/Light.cpp
  core/Light.hpp

  core/Transform.cpp
  core/Transform.hpp

  core/Window.cpp
  core/Window.hpp
)

set(SOURCE_RENDERER
  renderer/Context.cpp
  renderer/Context.hpp

  renderer/Material.cpp
  renderer/Material.hpp

  renderer/Model.cpp
  renderer/Model.hpp

  renderer/Renderer.cpp
  renderer/Renderer.hpp

  renderer/Settings.cpp
  renderer/Settings.hpp

  renderer/Shader.cpp
  renderer/Shader.hpp

  renderer/Sync.cpp
  renderer/Sync.hpp

  renderer/Texture.cpp
  renderer/Texture.hpp
)

set(SOURCE_RENDERER_BUFFERS
  renderer/buffers/Buffer.cpp
  renderer/buffers/Buffer.hpp

  renderer/buffers/Descriptor.cpp
  renderer/buffers/Descriptor.hpp
  
  renderer/buffers/DescriptorPool.cpp
  renderer/buffers/DescriptorPool.hpp

  renderer/buffers/IndexBuffer.cpp
  renderer/buffers/IndexBuffer.hpp

  renderer/buffers/UniformBuffer.cpp
  renderer/buffers/UniformBuffer.hpp

  renderer/buffers/VertexBuffer.cpp
  renderer/buffers/VertexBuffer.hpp
)

set(SOURCE_RENDERER_COMPOSITE_PASS
  renderer/composite_pass/CompositePipeline.cpp
  renderer/composite_pass/CompositePipeline.hpp

  renderer/composite_pass/Swapchain.cpp
  renderer/composite_pass/Swapchain.hpp

  renderer/composite_pass/UI.cpp
  renderer/composite_pass/UI.hpp
)

set(SOURCE_RENDERER_GEOMETRY_PASS
  renderer/geometry_pass/GeometryBuffer.cpp
  renderer/geometry_pass/GeometryBuffer.hpp

  renderer/geometry_pass/GeometryPipeline.cpp
  renderer/geometry_pass/GeometryPipeline.hpp
)

set(SOURCE_RENDERER_LIGHTING_PASS
  renderer/lighting_pass/LightingBuffer.cpp
  renderer/lighting_pass/LightingBuffer.hpp

  renderer/lighting_pass/LightingPipelines.cpp
  renderer/lighting_pass/LightingPipelines.hpp
)

set(SOURCE_RENDERER_SHADOW_PASS
  renderer/shadow_pass/ShadowMap.cpp
  renderer/shadow_pass/ShadowMap.hpp

  renderer/shadow_pass/ShadowPipeline.cpp
  renderer/shadow_pass/ShadowPipeline.hpp
)

set(SOURCE_SHADERS
  shaders/CompositePass.frag
  shaders/CompositePass.vert

  shaders/GeometryPass.frag
  shaders/GeometryPass.vert

  shaders/LightingPassNoShadowMaps.frag
  shaders/LightingPassNoShadowMaps.vert

  shaders/LightingPassWithShadowMaps.frag
  shaders/LightingPassWithShadowMaps.vert

  shaders/ShadowPass.frag
  shaders/ShadowPass.vert

  shaders/UI.frag
  shaders/UI.vert
)

set(SOURCE_SHADER_INCLUDES
  shaders/Lighting.include
)

set(SOURCE
  ${SOURCE_CORE}
  ${SOURCE_RENDERER}
  ${SOURCE_RENDERER_BUFFERS}
  ${SOURCE_RENDERER_COMPOSITE_PASS}
  ${SOURCE_RENDERER_GEOMETRY_PASS}
  ${SOURCE_RENDERER_LIGHTING_PASS}
  ${SOURCE_RENDERER_SHADOW_PASS}
  ${SOURCE_SHADERS}
  ${SOURCE_SHADER_INCLUDES}
)

set(INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Vulkan_INCLUDE_DIRS}
)

set(DEPENDENCIES
  assimp
  glm
  imgui
  imguizmo
  sdl
  sdl-main
  stb
  ${Vulkan_LIBRARIES}
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DEPENDENCIES})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})

set(SHADERS
  CompositePass.frag
  CompositePass.vert
  
  GeometryPass.frag
  GeometryPass.vert
  
  LightingPassNoShadowMaps.frag
  LightingPassNoShadowMaps.vert
  
  LightingPassWithShadowMaps.frag
  LightingPassWithShadowMaps.vert
  
  ShadowPass.frag
  ShadowPass.vert
  
  UI.frag
  UI.vert
)

foreach(SHADER ${SOURCE_SHADERS})
  set(SHADER_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
  set(SHADER_OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER}.spv)
  add_custom_command(
    OUTPUT ${SHADER_OUTPUT}
    COMMAND glslc ${SHADER_INPUT} -o ${SHADER_OUTPUT}
    MAIN_DEPENDENCY ${SHADER_INPUT}
    COMMENT "Compiling ${SHADER}"
    VERBATIM
  )
endforeach()

set(COPY_DIRECTORIES
  ${PROJECT_SOURCE_DIR}/models
  ${PROJECT_SOURCE_DIR}/textures
)

install(DIRECTORY ${COPY_DIRECTORIES} DESTINATION $<TARGET_FILE_DIR:${PROJECT_NAME}>)
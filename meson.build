project('Makma', 'cpp')

sdldep = dependency('sdl2')
vulkandep = dependency('vulkan')
assimpdep = dependency('assimp')

#hack: glm included because several source files include glm headers without glm/ dir
incdir = include_directories(['External/stb', 'External/GLM/glm', 'External/imgui'])

sources = [
  'Source/Camera.cpp',
  'Source/Game.cpp',
  'Source/Input.cpp',
  'Source/Light.cpp',
  'Source/Renderer/Buffers/Buffer.cpp',
  'Source/Renderer/Buffers/Descriptor.cpp',
  'Source/Renderer/Buffers/DescriptorPool.cpp',
  'Source/Renderer/Buffers/IndexBuffer.cpp',
  'Source/Renderer/Buffers/UniformBuffer.cpp',
  'Source/Renderer/Buffers/VertexBuffer.cpp',
  'Source/Renderer/Context.cpp',
  'Source/Renderer/GeometryPass/GeometryBuffer.cpp',
  'Source/Renderer/GeometryPass/GeometryPipeline.cpp',
  'Source/Renderer/LightingPass/LightingPipeline.cpp',
  'Source/Renderer/LightingPass/Swapchain.cpp',
  'Source/Renderer/Material.cpp',
  'Source/Renderer/Model.cpp',
  'Source/Renderer/Renderer.cpp',
  'Source/Renderer/Semaphores.cpp',
  'Source/Renderer/Shader.cpp',
  'Source/Renderer/ShadowPass/ShadowMap.cpp',
  'Source/Renderer/ShadowPass/ShadowPipeline.cpp',
  'Source/Renderer/Texture.cpp',
  'Source/Transform.cpp',
  'Source/Window.cpp'
]

executable('makma', sources, include_directories : incdir, dependencies : [sdldep, vulkandep, assimpdep])

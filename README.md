# Makma

![Screenshot](Screenshot.png)

Makma is a deferred renderer implemented entirely in Vulkan. It's written in modern C++ (using smart pointers, deleter lamdas etc.) and GLSL. It requires SDL2 for window creation and input handling, SDL_image for texture file, assimp for 3D model file loading and GLM as a math library. It features some preprocessor macro definitions that enable or disable certain Vulkan optimizations (such as buffer staging or push constants versus dynamic uniform buffers for example).

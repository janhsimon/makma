## What is Makma?

Makma is a Vulkan benchmark. It features a four-pass deferred renderer, cascaded shadow mapping, metal/roughness material setup, volumetric lighting, bloom, directional, point and spotlights, in-depth performance graphs (per pass) and a large options menu to customize the benchmark.

![Screenshot](Screenshot.png)


## How do I run Makma?

Grab the latest release from [here](https://github.com/janhsimon/makma/releases).

Here are the system requirements:

| **Category**         | **Requirement**          |
| -------------------- | ------------------------ |
| Operating System     | Windows or Linux, 64-bit |
| Graphics Card Memory | 2GB or more              |
| Graphics Driver      | Vulkan 1.2 support       |

Please note that the only Linux distribution I have tested is Ubuntu 20.04.


## How do I build Makma?

Makma is set up as a simple CMake project. All required libraries are supplied in the `external` folder of this repository.

In addition to the system requirements for running Makma, you'll need:

| **Category**         | **Requirement**           |
| -------------------- | ------------------------- |
| Graphics API         | Vulkan 1.2.170 SDK        |
| Build Generator      | CMake 3.2 or newer        |
| C++ Compiler         | Windows: MSVC 19.28.29914 |
|                      | Linux: GCC 9.3.0          |

Other Vulkan SDK versions may work, but have required significant code changes in the past. Other C++17-capable compilers or recent versions of MSVC or GCC should work as well.

Make sure to build the `makma` CMake target first, then `INSTALL` to copy the required files (shared libraries and program resources) into your build folder.

# vlkn

A real-time 3D renderer in C++20 on Vulkan, GLFW and GLM: textured, lit OBJ scenes with a free-flying quaternion camera and an ImGui debug overlay.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how the classes fit together and [docs/RENDERING_PIPELINE.md](docs/RENDERING_PIPELINE.md) for pipeline, descriptor and synchronization details.

## Project showcase

Playlist with demos: https://video.infosec.exchange/w/p/oRWze5L2wpb8tULWdEon8G

## Features

- **Frame loop:** swap-chain recreation on window resize and on `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR`, MAILBOX present mode with FIFO fallback, and two frames in flight synchronized with fences and semaphores.
- **One render pass, three pipelines:** opaque textured geometry, alpha-blended point-light billboards sorted back to front, and the ImGui overlay, all recorded into a single render pass with a depth buffer.
- **Resources:** OBJ loading with vertex deduplication (tinyobjloader); vertex, index and texture data uploaded through staging buffers into device-local memory; builder-style descriptor set layout, pool and writer wrappers.
- **Texturing:** images loaded with stb_image into an 8-slot combined-image-sampler array with anisotropic filtering; each object picks its texture through an index packed into its push constants.
- **Lighting:** per-fragment Blinn-Phong shading (ambient, diffuse and specular) with inverse-square attenuation from 16 orbiting point lights with interpolated rainbow colors and pulsing intensity, stored in a per-frame uniform buffer.
- **Camera:** 6-DOF free camera with quaternion orientation (yaw, pitch and roll without gimbal lock), mouse look, scroll-wheel field-of-view zoom and lock-on: number keys turn the camera to face a scene object.
- **Fixed timestep:** keyboard movement runs in an accumulator loop at 512 Hz, independent of the render frame rate.
- **ImGui overlay:** frame time and FPS, camera orientation and a live point-light color picker, built through my [fork of cmake-imgui](https://github.com/mikolajlubiak/cmake-imgui) with added Vulkan and GLFW backend support.

## Tech Stack

`C++20` `Vulkan` `GLSL / SPIR-V` `GLFW` `GLM` `Dear ImGui` `CMake` `Ninja`

## Build

Tested on Linux.

- Linux (and other Unix systems like macOS):
  - Install the dependencies (package names differ per distribution):
    - Fedora:
      - `sudo dnf install cmake git ninja-build vulkan-devel glfw-devel glm-devel glslang`
    - Arch:
      - `sudo pacman -S --needed cmake git ninja vulkan-headers vulkan-icd-loader glfw glm glslang`
    - Ubuntu:
      - `sudo apt install cmake git ninja-build libvulkan-dev libglfw3-dev libglm-dev glslang-tools`
  - `git clone --recurse-submodules https://github.com/mikolajlubiak/vlkn`
  - `cd vlkn`
  - `cmake -S cmake-imgui -B cmake-imgui/build -DCMAKE_INSTALL_PREFIX=cmake-imgui/build/dist && cmake --build cmake-imgui/build --target install`
  - `cmake --preset=default` (or `--preset=release` for an optimized build without validation layers)
  - `cmake --build build`
  - `cd build && ./vlkn` (shaders, models and textures are loaded relative to the working directory)
- Windows (WSL):
  - Set up WSL (Windows Subsystem for Linux)
  - Inside your WSL environment, follow the Linux steps

## Usage

- `W` `A` `S` `D`: move forward, left, backward and right
- `E` / `Q`: move up / down
- `Z` / `X`: roll left / right
- Mouse: look around
- Scroll wheel: zoom (field of view)
- `1`–`8`: turn the camera to face scene object 1–8
- `C`: release the cursor to use the ImGui overlay (press again to return to the camera)
- `Escape`: quit

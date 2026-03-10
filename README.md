# vlkn

A Vulkan-based 3D renderer and game engine written in C++20. This project demonstrates a complete Vulkan rendering pipeline including physical/logical device management, swap chain handling, a multi-pipeline render system, OBJ model loading, a 6-DOF camera, dynamic point lights with Phong shading, texture sampling, and an ImGui debug overlay. It is structured as a set of thin, focused abstractions over the Vulkan API, keeping each concern in its own class while remaining easy to follow.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for a detailed breakdown of the engine architecture and component relationships.

## Features

- **Vulkan rendering pipeline** — full graphics pipeline setup with configurable `PipelineConfigInfo`, SPIR-V shader loading, and dynamic viewport/scissor state
- **Swap chain management** — double-buffered swap chain with automatic recreation on window resize, surface format and present mode selection
- **Multi-pass rendering** — separate render systems for opaque geometry (textured), point light billboards (alpha-blended), and the ImGui overlay
- **OBJ model loading** — vertex and index buffer construction from OBJ files using tinyobjloader, with vertex deduplication via an unordered map
- **Texture sampling** — JPEG texture loading with mipmapping and anisotropic filtering; array of up to 8 combined image samplers bound per descriptor set
- **6-DOF camera system** — perspective projection, YXZ Euler-angle view matrix, independent keyboard (WASD + EQ + arrows + ZX) and mouse look/scroll-to-zoom controllers running at a fixed 512 Hz tick rate
- **Dynamic point lights** — up to 16 rainbow-coloured point lights orbiting the scene with sinusoidal intensity variation; back-to-front sorted for correct alpha blending
- **Blinn-Phong shading** — per-fragment ambient + diffuse + specular lighting with distance attenuation computed in the fragment shader
- **Push constants** — per-object model and normal matrices (render system) and per-light position/colour (point light system) passed via `vkCmdPushConstants`
- **Descriptor set management** — global UBO (projection/view matrices + light array) and a combined image sampler array bound once per frame through a single descriptor set
- **ImGui debug overlay** — real-time camera rotation display and point-light colour picker rendered within the shared render pass
- **Fixed-timestep game loop** — accumulator-based update loop decoupled from render frame rate

## Tech Stack

`C++` `Vulkan` `GLSL` `GLFW` `GLM` `ImGui` `CMake` `Ninja`

## Build

Tested on Linux.

- Linux (and other Unix systems like MacOS):
  - Install necessary packages (different commands based on your distribution)
    - Fedora:
      - `sudo dnf install cmake git ninja-build vulkan-devel glfw-devel glm-devel glslang`
    - Arch:
      - `sudo pacman -S --needed cmake git ninja vulkan-headers vulkan-icd-loader glfw glm glslang`
    - Ubuntu:
      - `sudo apt install cmake git ninja-build libvulkan-dev libglfw3-dev libglm-dev glslang-tools`
  - `git clone --recurse-submodules https://github.com/mikolajlubiak/vlkn`
  - `cd vlkn`
  - `cmake -S cmake-imgui -B cmake-imgui/build -DCMAKE_INSTALL_PREFIX=cmake-imgui/build/dist && cmake --build cmake-imgui/build --target install`
  - `cmake --preset=default`
  - `cmake --build build`
  - `./build/vlkn`
- Windows (WSL):
  - Setup WSL (Windows Subsystem for Linux)
  - Inside your WSL container do the Linux steps

## Usage

- Use the keyboard to move the camera:
  - `WASD` — move forward/backward/left/right
  - `E` / `Q` — move up/down
  - Arrow keys — look around
  - `Z` / `X` — roll left/right
  - `Escape` — close the app
- Use the mouse to look around

# Contributing

Thank you for your interest in contributing to **vlkn**. This document explains how to set up the development environment, follow the project's code conventions, and submit changes.

---

## Table of Contents

1. [Development environment setup](#development-environment-setup)
2. [Project structure](#project-structure)
3. [Code style guidelines](#code-style-guidelines)
4. [Submitting changes](#submitting-changes)

---

## Development environment setup

### Prerequisites

| Tool | Minimum version | Purpose |
|------|----------------|---------|
| GCC or Clang | C++20 support | Compiler |
| CMake | 3.5 | Build system |
| Ninja | any | Build backend |
| Vulkan SDK / headers | 1.2+ | Rendering API |
| GLFW | 3.x | Window and input |
| GLM | 0.9.9+ | Mathematics |
| glslang / `glslangValidator` | any | GLSL → SPIR-V compilation |

ImGui is included as a git submodule under `cmake-imgui/` and is built locally; it does not need to be installed system-wide.

### Install dependencies

**Fedora**
```bash
sudo dnf install cmake git ninja-build vulkan-devel glfw-devel glm-devel glslang
```

**Arch Linux**
```bash
sudo pacman -S --needed cmake git ninja vulkan-headers vulkan-icd-loader glfw glm glslang
```

**Ubuntu / Debian**
```bash
sudo apt install cmake git ninja-build libvulkan-dev libglfw3-dev libglm-dev glslang-tools
```

### Clone and build

```bash
# 1. Clone with submodules
git clone --recurse-submodules https://github.com/mikolajlubiak/vlkn
cd vlkn

# 2. Build and install ImGui (done once)
cmake -S cmake-imgui -B cmake-imgui/build \
      -DCMAKE_INSTALL_PREFIX=cmake-imgui/build/dist
cmake --build cmake-imgui/build --target install

# 3. Configure the main project
cmake --preset=default

# 4. Build
cmake --build build

# 5. Run
./build/vlkn
```

The `default` preset (defined in `CMakePresets.json`) enables Ninja and a debug build. For an optimised build, pass `-DCMAKE_BUILD_TYPE=Release` to step 3.

### Shader compilation

GLSL shaders in `shaders/` are compiled to SPIR-V automatically as part of the CMake build via `glslangValidator`. The compiled `.spv` files are placed in `build/shaders/`. If you modify a shader, rebuild the project and the shader will be recompiled.

---

## Project structure

```
vlkn/
├── CMakeLists.txt          # Main build definition
├── CMakePresets.json       # Build presets (default = Ninja + Debug)
├── cmake-imgui/            # ImGui submodule (built separately)
├── docs/                   # Architecture and pipeline documentation
├── models/                 # OBJ mesh files loaded at runtime
├── shaders/                # GLSL source shaders (compiled to SPIR-V)
├── textures/               # Texture images loaded at runtime
└── src/
    ├── main.cpp                          # Entry point
    ├── app.hpp / app.cpp                 # Application class and main loop
    ├── vlkn_window.hpp/cpp               # GLFW window abstraction
    ├── vlkn_device.hpp/cpp               # Vulkan instance/device/queues
    ├── vlkn_swap_chain.hpp/cpp           # Swap chain, framebuffers, sync
    ├── vlkn_pipeline.hpp/cpp             # Graphics pipeline creation
    ├── vlkn_renderer.hpp/cpp             # Command buffer lifecycle
    ├── vlkn_model.hpp/cpp                # OBJ loading, vertex/index buffers
    ├── vlkn_buffer.hpp/cpp               # GPU buffer abstraction
    ├── vlkn_image.hpp/cpp                # Texture image, sampler
    ├── vlkn_camera.hpp/cpp               # View/projection matrices
    ├── vlkn_game_object.hpp/cpp          # Entity with transform + optional components
    ├── vlkn_frame_info.hpp               # FrameInfo, GlobalUbo, PointLight structs
    ├── vlkn_descriptors.hpp/cpp          # Descriptor set layout, pool, writer
    ├── vlkn_utils.hpp                    # Hash helpers
    ├── keyboard_movement_controller.hpp/cpp  # Keyboard camera control
    ├── mouse_movement_controller.hpp/cpp     # Mouse look + scroll zoom
    └── systems/
        ├── render_system.hpp/cpp         # Textured geometry rendering
        ├── point_light_system.hpp/cpp    # Point light billboards
        └── imgui_system.hpp/cpp          # ImGui debug overlay
```

---

## Code style guidelines

The codebase follows consistent conventions throughout. Please match the style of the surrounding code when making changes.

### Naming

| Construct | Convention | Example |
|-----------|-----------|---------|
| Classes | `PascalCase` with `Vlkn` prefix for engine types | `VlknDevice`, `VlknBuffer` |
| Member variables | `camelCase` | `vlknDevice`, `currentFrameIndex` |
| Functions / methods | `camelCase` | `beginFrame()`, `createSwapChain()` |
| Constants / `constexpr` | `SCREAMING_SNAKE_CASE` | `MAX_FRAMES_IN_FLIGHT`, `MAX_LIGHTS` |
| Local variables | `camelCase` | `frameIndex`, `pushConstantRange` |
| Struct fields | `camelCase` | `modelMatrix`, `lightIntensity` |
| Namespaces | `lowercase` | `namespace vlkn` |

### File layout

Each header uses `#pragma once`. Includes are grouped and ordered:

```cpp
// header (own .hpp)
#include "this_file.hpp"

// local
#include "vlkn_device.hpp"
#include "vlkn_model.hpp"

// libs
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>
```

### Class structure

- Copy constructor and copy assignment operator are `= delete` on all resource-owning classes.
- Move semantics are not explicitly defined; resources are managed through `std::unique_ptr` / `std::shared_ptr`.
- Public interface first, `private` implementation details last.

### Vulkan handle ownership

Each class owns the Vulkan handles it creates and destroys them in its destructor. Never store raw Vulkan handles outside the class responsible for their lifetime.

### Builder pattern

Use the builder pattern (as in `VlknDescriptorSetLayout::Builder` and `VlknDescriptorPool::Builder`) when constructing objects that require multiple optional configuration steps before creation.

### Error handling

Vulkan calls that return `VkResult` are checked immediately. On failure, throw `std::runtime_error` with a descriptive message:

```cpp
if (vkCreateFoo(device, &info, nullptr, &handle) != VK_SUCCESS) {
    throw std::runtime_error("failed to create foo");
}
```

### Push constants and UBO structs

Keep GPU-facing structs in the file that uses them (`PushConstantData` inside `render_system.cpp`, `GlobalUbo` inside `vlkn_frame_info.hpp`). Align fields to match GLSL std140/std430 requirements.

---

## Submitting changes

1. **Fork** the repository and create a branch from `main`:
   ```bash
   git checkout -b feature/my-improvement
   ```

2. **Make your changes**, following the code style guidelines above.

3. **Test locally**: build the project and run `./build/vlkn` to confirm it compiles and runs correctly. The project has no automated test suite; manual verification is expected.

4. **Commit** with a clear, concise message describing *what* changed and *why*:
   ```
   Add mipmap generation for loaded textures

   Previously, VlknImage only created a single mip level. This commit adds
   vkCmdBlitImage-based mipmap generation during image upload to improve
   texture quality at distance.
   ```

5. **Open a pull request** against `main`. Include a brief description of the change and, if applicable, screenshots showing the visual result.

### Commit message style

- Use the imperative mood in the subject line: "Add", "Fix", "Remove", "Refactor".
- Keep the subject line under 72 characters.
- Add a blank line between the subject and body.
- The body should explain the motivation and any non-obvious implementation choices.

### What makes a good contribution

- Stays focused: one logical change per pull request.
- Does not break the existing build or introduce new compiler warnings.
- Follows the naming and ownership conventions above.
- Includes updated documentation in `docs/` when adding or significantly changing a subsystem.

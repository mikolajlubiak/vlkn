# vlkn
yet another Vulkan renderer

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

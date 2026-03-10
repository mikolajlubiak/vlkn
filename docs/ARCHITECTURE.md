# Architecture

This document describes the high-level design of **vlkn** — a Vulkan 3D renderer built in C++20. The goal is to give a reader enough context to navigate the source code confidently, understand how the pieces fit together, and extend or modify the engine without surprises.

---

## Overview

vlkn is structured as a set of single-responsibility classes that wrap the Vulkan API at progressively higher levels of abstraction. Each class owns the Vulkan handles it creates and destroys them in its destructor, making lifetime management explicit and leak-free. No global state is used; everything flows through constructor arguments or the `FrameInfo` struct passed to render systems each frame.

The application layer (`App`) owns one instance of each major subsystem and drives the main loop. Render systems are stateless workers that receive a `FrameInfo` reference every frame and record draw commands into the active command buffer.

---

## Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                          App (app.hpp)                          │
│  main loop · loadGameObjects · fixed-timestep update at 512 Hz  │
└───┬───────────┬──────────────┬───────────────┬─────────────────┘
    │           │              │               │
    ▼           ▼              ▼               ▼
┌────────┐ ┌─────────┐ ┌────────────┐ ┌──────────────────────┐
│VlknWin │ │VlknDev  │ │VlknRenderer│ │  VlknDescriptorPool  │
│(GLFW   │ │(physical│ │(command    │ │  VlknDescriptorSet   │
│ window,│ │ device, │ │ buffers,   │ │  Layout              │
│ surface│ │ logical │ │ render     │ │  VlknDescriptorWriter│
│ resize │ │ device, │ │ pass mgmt, │ └──────────┬───────────┘
│ events)│ │ queues, │ │ swap chain │             │
└────────┘ │ mem alloc│ │ recreation)│             │ global UBO +
           │ cmd pools│ └─────┬──────┘             │ sampler array
           └─────────┘       │                     │
                             ▼                     │
                    ┌─────────────────┐            │
                    │  VlknSwapChain  │            │
                    │ (image acquire, │            │
                    │  present,       │            │
                    │  framebuffers,  │            │
                    │  depth images,  │            │
                    │  semaphores,    │            │
                    │  fences)        │            │
                    └─────────────────┘            │
                                                   │
         ┌──────────────────────────────────────── ┘
         │
         ▼  FrameInfo (command buffer + camera + descriptor set + game objects)
┌────────────────────────────────────────────────────────────────┐
│                       Render Systems                           │
│                                                                │
│  ┌──────────────────┐  ┌───────────────────┐  ┌────────────┐  │
│  │   RenderSystem   │  │ PointLightSystem  │  │ImGuiSystem │  │
│  │ (textured OBJ    │  │ (billboard quads, │  │(debug UI,  │  │
│  │  geometry,       │  │  alpha blending,  │  │ colour     │  │
│  │  Blinn-Phong     │  │  back-to-front    │  │ picker,    │  │
│  │  lighting,       │  │  sorting,         │  │ rotation   │  │
│  │  push constants) │  │  push constants)  │  │ display)   │  │
│  └────────┬─────────┘  └─────────┬─────────┘  └─────┬──────┘  │
└───────────┼─────────────────────┼────────────────────┼─────────┘
            │                     │                    │
            ▼                     ▼                    │
    ┌───────────────┐    ┌─────────────────┐           │
    │  VlknPipeline │    │  VlknPipeline   │           │
    │(render_textured│   │ (point_light    │           │
    │ vert/frag spv) │   │  vert/frag spv) │           │
    └───────┬────────┘   └────────┬────────┘           │
            │                    │                     │
            ▼                    ▼                     ▼
    ┌──────────────────────────────────────────────────────────┐
    │                    Scene Objects                         │
    │                                                          │
    │  VlknGameObject::Map (unordered_map<id_t, GameObject>)   │
    │   ├── flat_vase  (VlknModel + TransformComponent)        │
    │   ├── smooth_vase(VlknModel + TransformComponent)        │
    │   ├── floor quad (VlknModel + TransformComponent)        │
    │   └── [0..15] point lights (PointLightComponent + color) │
    └───────────────────┬──────────────────────────────────────┘
                        │
            ┌───────────┴────────────┐
            ▼                        ▼
     ┌─────────────┐        ┌──────────────┐
     │  VlknModel  │        │  VlknBuffer  │
     │(OBJ loader, │        │(GPU buffer,  │
     │ vertex buf, │        │ memory map,  │
     │ index buf)  │        │ flush,       │
     └─────────────┘        │ descriptor)  │
                            └──────────────┘

    ┌────────────────────────────────────────┐
    │         Input / Camera                 │
    │                                        │
    │  KeyboardMovementController            │
    │   └─ WASD move, arrow look, ZX roll    │
    │  MouseMovementController               │
    │   └─ mouse look, scroll FOV zoom       │
    │  VlknCamera                            │
    │   └─ YXZ view matrix, perspective proj │
    └────────────────────────────────────────┘

    ┌────────────────────────────────────────┐
    │         Resource Management            │
    │                                        │
    │  VlknImage  (texture load, sampler,    │
    │              layout transitions)       │
    │  VlknBuffer (vertex, index, UBO)       │
    └────────────────────────────────────────┘
```

---

## Component Descriptions

### App (`src/app.hpp`, `src/app.cpp`)

The top-level class that owns every subsystem. Its constructor builds the descriptor pool and descriptor set layout, allocates per-frame UBO buffers, loads textures, writes descriptor sets, and creates the three render systems. The `run()` method is the main loop: it polls GLFW events, runs the fixed-timestep input update at 512 Hz, updates the camera, fills the `GlobalUbo` struct, and drives the renderer's begin/end frame lifecycle. `loadGameObjects()` populates the `VlknGameObject::Map` with the two vases, the floor quad, and sixteen rainbow point lights arranged in a circle.

### VlknWindow (`src/vlkn_window.hpp`, `src/vlkn_window.cpp`)

A thin wrapper around a `GLFWwindow`. Handles window creation, the Vulkan surface (`VkSurfaceKHR`), mouse input (raw cursor delta via `GLFW_RAW_MOUSE_MOTION`), and window resize callbacks. Exposes `shouldClose()`, `getExtent()`, and `wasWindowResized()` so the renderer can detect when to recreate the swap chain.

### VlknDevice (`src/vlkn_device.hpp`, `src/vlkn_device.cpp`)

Manages the Vulkan instance, debug messenger, physical device selection, logical device, graphics/present queues, command pool, and a single-use command buffer helper for staging operations. Physical device selection prefers a dedicated GPU and verifies required extensions (`VK_KHR_swapchain`) and swap chain support. Validation layers and `VK_EXT_debug_utils` are enabled in debug builds via the `APP_USE_VULKAN_DEBUG_REPORT` define.

### VlknSwapChain (`src/vlkn_swap_chain.hpp`, `src/vlkn_swap_chain.cpp`)

Owns the `VkSwapchainKHR`, swap chain images and image views, per-frame depth image/view/memory, the `VkRenderPass`, framebuffers, and all synchronisation objects (two `imageAvailableSemaphores`, two `renderFinishedSemaphores`, per-frame in-flight fences, and per-image fences to prevent presenting an image still being rendered). The constructor accepts an optional `shared_ptr<VlknSwapChain>` for the old swap chain to enable seamless recreation. `MAX_FRAMES_IN_FLIGHT = 2` limits CPU/GPU pipelining to two frames.

### VlknRenderer (`src/vlkn_renderer.hpp`, `src/vlkn_renderer.cpp`)

Manages the `VkCommandBuffer` array (one per frame in flight) and owns `VlknSwapChain`. Provides the four-function rendering lifecycle: `beginFrame()` → `beginSwapChainRenderPass()` → (render systems record commands) → `endSwapChainRenderPass()` → `endFrame()`. When `vkAcquireNextImageKHR` or `vkQueuePresentKHR` returns `VK_ERROR_OUT_OF_DATE_KHR` or `VK_SUBOPTIMAL_KHR`, `recreateSwapChain()` is called automatically.

### VlknPipeline (`src/vlkn_pipeline.hpp`, `src/vlkn_pipeline.cpp`)

Loads SPIR-V bytecode from disk, creates `VkShaderModule` objects, and builds a `VkPipeline` from a `PipelineConfigInfo` struct. `defaultPipelineConfigInfo()` sets up triangle-list topology, fill-mode rasterization, no multisampling, depth test + write enabled, and dynamic viewport/scissor. `enableAlphaBlending()` switches the colour blend attachment to standard src-alpha / one-minus-src-alpha blending (used for the point light billboards).

### RenderSystem (`src/systems/render_system.hpp`, `src/systems/render_system.cpp`)

Creates the textured geometry pipeline (`render_textured.vert/frag`). Each frame it binds the pipeline and global descriptor set, then iterates the game object map. For every object with a non-null model it writes a `PushConstantData` struct containing the 4×4 model matrix and the 4×4 normal matrix (with the texture index packed into `[3][3]`), then calls `vkCmdPushConstants` followed by `model->bind()` and `model->draw()`.

### PointLightSystem (`src/systems/point_light_system.hpp`, `src/systems/point_light_system.cpp`)

Creates the point light billboard pipeline (`point_light.vert/frag`) with alpha blending enabled and no vertex input (six hardcoded vertices form a billboard quad in the vertex shader). The `update()` method rotates all lights around the Y axis each frame and modulates their intensity with a sine wave. The `render()` method sorts lights back-to-front by camera distance so alpha blending composites correctly, then issues one `vkCmdDraw(6, 1, ...)` per light with position/colour in push constants.

### ImGuiSystem (`src/systems/imgui_system.hpp`, `src/systems/imgui_system.cpp`)

Initialises ImGui for Vulkan using the helper from the `cmake-imgui` submodule (built and installed separately). Exposes `update()` to build the ImGui frame (camera rotation angles, point light colour picker) and `render()` to record the ImGui draw data into the command buffer. The colour returned by `getPointLightColor()` is consumed by both the `PointLightSystem` update and render calls.

### VlknDescriptors (`src/vlkn_descriptors.hpp`, `src/vlkn_descriptors.cpp`)

Three cooperating classes:

- **`VlknDescriptorSetLayout`** — builder pattern that accumulates `VkDescriptorSetLayoutBinding` entries and produces a `VkDescriptorSetLayout`.
- **`VlknDescriptorPool`** — builder pattern that allocates a `VkDescriptorPool` sized for the expected number of sets and pool sizes.
- **`VlknDescriptorWriter`** — writes buffer info and image info into a descriptor set without requiring the caller to manage `VkWriteDescriptorSet` directly. Supports writing arrays of image descriptors (`writeImageArray`) for the texture sampler array.

### VlknModel (`src/vlkn_model.hpp`, `src/vlkn_model.cpp`)

Loads OBJ files using `tinyobjloader`. Vertices are deduplicated using an `std::unordered_map<Vertex, uint32_t>` with a custom hash. Separate `VlknBuffer` objects are created for the vertex buffer and index buffer; both use a staging buffer (host-visible) that is copied to device-local memory for optimal GPU access. Exposes `bind()` (binds vertex and index buffers) and `draw()` (issues `vkCmdDrawIndexed`).

### VlknBuffer (`src/vlkn_buffer.hpp`, `src/vlkn_buffer.cpp`)

A general-purpose GPU buffer wrapper. Supports persistent mapping (`map()`/`unmap()`), writing (`writeToBuffer()`), flushing non-coherent memory ranges (`flush()`), and generating a `VkDescriptorBufferInfo` for descriptor set writes. The alignment calculation for uniform buffers uses the device's `minUniformBufferOffsetAlignment`.

### VlknImage (`src/vlkn_image.hpp`, `src/vlkn_image.cpp`)

Loads JPEG/PNG images from disk using `stb_image`, uploads them via a staging buffer, generates mipmaps with `vkCmdBlitImage`, and creates a `VkImageView` and `VkSampler` with anisotropic filtering. Also provides `createEmptyImage()` for placeholder slots in the texture array. Handles image layout transitions via pipeline barriers.

### VlknCamera (`src/vlkn_camera.hpp`, `src/vlkn_camera.cpp`)

Provides `setViewYXZ()` (builds the view matrix from a translation and YXZ Euler rotation) and `setPerspectiveProjection()` (standard perspective matrix with Y flipped for Vulkan's coordinate system). Also exposes `getPosition()` (derived from the inverse view matrix) for distance-sorted light rendering.

### KeyboardMovementController / MouseMovementController

`KeyboardMovementController` maps GLFW key states to camera translation and rotation deltas applied at a fixed tick rate. It also implements a "look at nearest object" feature triggered by a key press. `MouseMovementController` computes yaw/pitch from raw mouse delta and adjusts the field of view with the scroll wheel.

### VlknGameObject / TransformComponent (`src/vlkn_game_object.hpp`)

`VlknGameObject` is a simple entity with an auto-incremented integer ID, an optional shared `VlknModel`, a `TransformComponent` (translation, rotation, scale), an optional `PointLightComponent`, a colour, and a texture index (`imgIdx`). `TransformComponent::mat4()` builds the TRS matrix and `normalMatrix()` returns the transpose-inverse for correct normal transformation.

---

## Rendering Flow

A single frame proceeds as follows:

```
1. glfwPollEvents()
   │
2. Fixed-timestep update loop (512 Hz)
   │  keyboardController.move(tickrate)
   │
3. mouseController.lookAround()
   keyboardController.lookAt(gameObjects)   // optional snap-to-object
   camera.setViewYXZ(...)
   camera.setPerspectiveProjection(...)
   │
4. vlknRenderer.beginFrame()
   │  vkAcquireNextImageKHR → currentImageIndex
   │  vkBeginCommandBuffer(commandBuffers[frameIndex])
   │  → returns commandBuffer (or nullptr if swap chain needs recreation)
   │
5. Update stage (CPU-side, before recording draw commands)
   │  pointLightSystem.update(frameInfo, lightColor, ubo)  // rotate lights
   │  uboBuffers[frameIndex]->writeToBuffer(&ubo)
   │  uboBuffers[frameIndex]->flush()
   │  imguiSystem.update(rotation)  // build ImGui widgets
   │
6. vlknRenderer.beginSwapChainRenderPass(commandBuffer)
   │  vkCmdBeginRenderPass → color clear + depth clear
   │  vkCmdSetViewport / vkCmdSetScissor
   │
7. renderSystem.renderGameObjects(frameInfo)
   │  bind pipeline (render_textured)
   │  bind global descriptor set (UBO + sampler array)
   │  for each game object with a model:
   │    vkCmdPushConstants(modelMatrix, normalMatrix + texIndex)
   │    vkCmdBindVertexBuffers / vkCmdBindIndexBuffer
   │    vkCmdDrawIndexed
   │
8. pointLightSystem.render(frameInfo, lightColor)
   │  sort lights back-to-front
   │  bind pipeline (point_light, alpha blend)
   │  bind global descriptor set
   │  for each light (back-to-front):
   │    vkCmdPushConstants(position + scale, color + intensity)
   │    vkCmdDraw(6 vertices)  // billboard generated in vertex shader
   │
9. imguiSystem.render(frameInfo)
   │  ImGui::Render()
   │  ImGui_ImplVulkan_RenderDrawData(...)
   │
10. vlknRenderer.endSwapChainRenderPass(commandBuffer)
    │  vkCmdEndRenderPass
    │
11. vlknRenderer.endFrame()
    │  vkEndCommandBuffer
    │  vkQueueSubmit (wait: imageAvailableSemaphore,
    │                 signal: renderFinishedSemaphore,
    │                 fence: inFlightFence)
    │  vkQueuePresentKHR (wait: renderFinishedSemaphore)
    │  → VK_ERROR_OUT_OF_DATE_KHR / VK_SUBOPTIMAL_KHR → recreateSwapChain()
```

---

## Key Design Decisions

**Single render pass, multiple pipelines**
All draw calls (geometry, point lights, ImGui) share one `VkRenderPass` with a single subpass. Separate `VkPipeline` objects handle the different shading requirements (textured Blinn-Phong vs. billboard quads vs. ImGui). This avoids subpass dependencies and keeps synchronisation simple.

**Push constants for per-object data**
Per-object model matrix, normal matrix, and texture index are delivered via push constants rather than a per-object UBO or dynamic descriptor. Push constants have the lowest latency of any Vulkan data-upload mechanism and require no buffer management for small per-draw payloads.

**Global UBO for shared per-frame data**
Projection/view matrices and the full point light array are written once per frame into a host-visible, persistently-mapped `VlknBuffer` and bound as a single descriptor set that all pipelines share. This avoids rebinding descriptors between draw calls.

**Staging buffers for GPU-local resources**
Vertex buffers, index buffers, and textures are first written into a host-visible staging buffer and then transferred to `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT` memory for optimal GPU access. The staging buffer is destroyed immediately after the transfer.

**Double buffering (MAX_FRAMES_IN_FLIGHT = 2)**
Two command buffers, two UBO buffers, and two descriptor sets are maintained so the CPU can record frame N+1 while the GPU renders frame N, without stalling.

**Fixed-timestep input at 512 Hz**
Camera movement is decoupled from frame rate using a standard accumulator loop. This gives deterministic, frame-rate-independent movement without requiring the render loop to run at a fixed rate.

**RAII ownership throughout**
Every Vulkan handle is owned by a C++ object. Constructors acquire resources, destructors release them. `std::unique_ptr` and `std::shared_ptr` are used for heap-allocated subsystems (`VlknSwapChain`, `VlknPipeline`, `VlknModel`). Copy constructors and copy assignment operators are deleted on all resource-owning classes.

**Builder pattern for descriptors**
`VlknDescriptorSetLayout::Builder` and `VlknDescriptorPool::Builder` collect configuration incrementally and produce the final Vulkan object in a single `build()` call. This makes descriptor set configuration readable at the call site without requiring the caller to manage intermediate Vulkan structs.

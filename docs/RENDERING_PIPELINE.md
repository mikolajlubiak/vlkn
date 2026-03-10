# Rendering Pipeline

This document describes the Vulkan rendering pipeline implementation in **vlkn** in detail: how the graphics pipelines are configured, how shaders are structured, how descriptors and push constants work, and how synchronisation and swap chain management are handled.

---

## Table of Contents

1. [Pipeline overview](#pipeline-overview)
2. [Shader stages](#shader-stages)
3. [Pipeline configuration](#pipeline-configuration)
4. [Descriptor set layout](#descriptor-set-layout)
5. [Push constants](#push-constants)
6. [Swap chain management and recreation](#swap-chain-management-and-recreation)
7. [Synchronisation](#synchronisation)

---

## Pipeline overview

vlkn uses **three separate `VkPipeline` objects** rendered in sequence within a single render pass:

```
Render Pass (single subpass)
│
├─── 1. RenderSystem pipeline         (render_textured.vert/frag)
│        Opaque textured geometry
│        Depth test ON, depth write ON
│        No blending
│
├─── 2. PointLightSystem pipeline     (point_light.vert/frag)
│        Billboard quads for light visualisation
│        Depth test ON, depth write OFF
│        Alpha blending (src_alpha / one_minus_src_alpha)
│        No vertex input (billboard generated in shader)
│
└─── 3. ImGui pipeline                (managed by ImGui Vulkan backend)
         UI overlay
         Alpha blending
```

All three pipelines share the same `VkRenderPass` and framebuffers. They are recorded into the same command buffer in order.

---

## Shader stages

### Textured geometry — `render_textured.vert` / `render_textured.frag`

**Vertex shader inputs (per vertex)**

| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec3` | `position` | Object-space position |
| 1 | `vec3` | `color` | Per-vertex colour tint |
| 2 | `vec3` | `normal` | Object-space normal |
| 3 | `vec2` | `uv` | Texture coordinates |

**Vertex shader outputs → fragment shader inputs**

| Location | Type | Name | Description |
|----------|------|------|-------------|
| 0 | `vec3` | `fragColor` | Passed-through vertex colour |
| 1 | `vec3` | `fragPosWorld` | World-space fragment position |
| 2 | `vec3` | `fragNormalWorld` | World-space surface normal |
| 3 | `vec2` | `fragUV` | Texture coordinates |

**Vertex shader transformation**

```glsl
vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
gl_Position = ubo.projection * ubo.view * positionWorld;
fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
```

The model matrix transforms from object space to world space. The view and projection matrices are from the global UBO. Normals are transformed using the upper-left 3×3 of the normal matrix (transpose-inverse of the model matrix) to handle non-uniform scaling correctly.

**Fragment shader — Blinn-Phong lighting**

```
outColor = (ambient + Σ diffuse_i + Σ specular_i) * fragColor * texColor
```

For each active point light:

1. **Attenuation**: `1 / dot(directionToLight, directionToLight)` — inverse square law
2. **Diffuse**: `lightContribution * max(dot(surfaceNormal, L), 0.0)`
3. **Specular** (Blinn-Phong): `lightContribution * pow(max(dot(N, H), 0.0), 512.0)` where `H` is the half-vector between the light direction and the view direction

The texture index is recovered from `push.normalMatrix[3][3]` (the `w`-component of the last column of the normal matrix, which is unused by the 3×3 transform and repurposed as a cheap per-object integer uniform).

---

### Point light billboard — `point_light.vert` / `point_light.frag`

The point light pipeline uses **no vertex buffer**. A hardcoded array of six `vec2` offsets defines a unit quad:

```glsl
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0),
  vec2(1.0, -1.0),  vec2(-1.0, 1.0), vec2(1.0, 1.0)
);
```

**Vertex shader — billboard projection**

```glsl
vec4 lightPosInCamera = ubo.view * vec4(push.position.xyz, 1.0);
// push.position.w holds the billboard radius (object scale)
vec4 positionInCamera = lightPosInCamera + push.position.w * vec4(fragOffset, 0.0, 0.0);
gl_Position = ubo.projection * positionInCamera;
```

The billboard is expanded in camera space so it always faces the viewer, regardless of camera orientation.

**Fragment shader — radial falloff**

```glsl
float distance = dot(fragOffset, fragOffset);  // squared distance from centre
if (distance > 1.0) discard;                   // clip to circle
float distanceCosine = 0.5 * (cos(sqrt(distance) * PI) + 0.5);
// rgb = (colored light) + (white specular core), alpha = soft edge
outColor = vec4(push.color.xyz * push.color.w + pow(distanceCosine, 8.0), distanceCosine);
```

Pixels outside the unit circle are discarded. Inside, the alpha and brightness follow a cosine curve peaking at the centre, giving a soft glow effect. The `pow(..., 8.0)` sharpens the highlight at the centre.

---

## Pipeline configuration

Both render system pipelines are created via `VlknPipeline`, which takes a `PipelineConfigInfo` struct. `VlknPipeline::defaultPipelineConfigInfo()` sets the following defaults:

```
Topology:          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
Polygon mode:      VK_POLYGON_MODE_FILL
Cull mode:         VK_CULL_MODE_NONE
Front face:        VK_FRONT_FACE_CLOCKWISE
Depth test:        ON  (VK_COMPARE_OP_LESS)
Depth write:       ON
Multisampling:     OFF (VK_SAMPLE_COUNT_1_BIT)
Dynamic states:    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
Color blending:    OFF (colour blend factor = ONE, alpha blend factor = ZERO)
```

`VlknPipeline::enableAlphaBlending()` modifies the colour blend attachment for the point light pipeline:

```
srcColorBlendFactor:  VK_BLEND_FACTOR_SRC_ALPHA
dstColorBlendFactor:  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
colorBlendOp:         VK_BLEND_OP_ADD
srcAlphaBlendFactor:  VK_BLEND_FACTOR_ONE
dstAlphaBlendFactor:  VK_BLEND_FACTOR_ZERO
alphaBlendOp:         VK_BLEND_OP_ADD
```

Depth writing is also disabled for the billboard pipeline so that light spheres do not occlude each other in the depth buffer.

### Vertex input

The textured geometry pipeline describes a single interleaved vertex binding at binding 0 with a per-vertex stride of `sizeof(VlknModel::Vertex)`:

| Location | Format | Offset | Description |
|----------|--------|--------|-------------|
| 0 | `VK_FORMAT_R32G32B32_SFLOAT` | 0 | `position` |
| 1 | `VK_FORMAT_R32G32B32_SFLOAT` | 12 | `color` |
| 2 | `VK_FORMAT_R32G32B32_SFLOAT` | 24 | `normal` |
| 3 | `VK_FORMAT_R32G32_SFLOAT` | 36 | `uv` |

The point light pipeline clears both `bindingDescriptions` and `attributeDescriptions` (no vertex buffer bound; all data comes from push constants and `gl_VertexIndex`).

---

## Descriptor set layout

A single global descriptor set layout is shared by all pipelines:

```
Set 0, Binding 0: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
  Stages: VERTEX | FRAGMENT
  Contents: GlobalUbo {
    mat4  projection;
    mat4  view;
    mat4  inverseView;
    vec4  ambientLightColor;    // xyz = colour, w = intensity
    PointLight pointLights[16]; // vec4 position + vec4 color each
    uint  lightsNum;
  }

Set 0, Binding 1: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER (array of 8)
  Stages: FRAGMENT
  Contents: 8 texture slots
    [0] = empty/placeholder image
    [1..7] = loaded texture(s)
```

The global UBO is written once per frame (after the `PointLightSystem::update()` call updates light positions) and uploaded via a persistently-mapped host-visible `VlknBuffer`. The descriptor set is bound once per pipeline with `vkCmdBindDescriptorSets` before all draw calls for that pipeline.

Each texture in the sampler array was loaded from disk and uploaded to a device-local `VkImage` with `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`. The sampler uses trilinear filtering (`VK_FILTER_LINEAR` + `VK_SAMPLER_MIPMAP_MODE_LINEAR`) and anisotropic filtering up to the device maximum.

---

## Push constants

### RenderSystem — per-object geometry data

```glsl
layout(push_constant) uniform Push {
    mat4 modelMatrix;    // 64 bytes
    mat4 normalMatrix;   // 64 bytes — [3][3] repurposed as texture index
} push;
// Total: 128 bytes
```

Push constant stage flags: `VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT`

The texture array index (`imgIdx` on `VlknGameObject`) is packed into the unused `[3][3]` element of the normal matrix before the `vkCmdPushConstants` call:

```cpp
push.normalMatrix = glm::mat4(obj.transform.normalMatrix());
push.normalMatrix[3][3] = obj.imgIdx;
```

### PointLightSystem — per-light billboard data

```glsl
layout(push_constant) uniform Push {
    vec4 position;  // xyz = world position, w = billboard radius
    vec4 color;     // xyz = colour, w = intensity
} push;
// Total: 32 bytes
```

Push constant stage flags: `VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT`

One `vkCmdPushConstants` + `vkCmdDraw(6, 1, 0, 0)` is issued per light, in back-to-front sorted order.

---

## Swap chain management and recreation

### Initial creation

`VlknSwapChain` queries the physical device for surface capabilities, available formats, and present modes. Selection logic:

- **Surface format**: prefers `VK_FORMAT_B8G8R8A8_SRGB` / `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`; falls back to the first available format.
- **Present mode**: prefers `VK_PRESENT_MODE_MAILBOX_KHR` (triple buffering, no tearing, low latency); falls back to `VK_PRESENT_MODE_FIFO_KHR` (vsync, always available).
- **Swap extent**: uses the window's framebuffer pixel size, clamped to the surface's `minImageExtent` / `maxImageExtent`.
- **Image count**: `minImageCount + 1`, clamped to `maxImageCount`.

The swap chain is created with `VK_SHARING_MODE_EXCLUSIVE` when the graphics and present queues are the same family (typical on most hardware), or `VK_SHARING_MODE_CONCURRENT` otherwise.

### Depth buffer

A depth image is created for each swap chain image using the best available depth format selected by `findDepthFormat()` (prefers `VK_FORMAT_D32_SFLOAT`, then `VK_FORMAT_D32_SFLOAT_S8_UINT`, then `VK_FORMAT_D24_UNORM_S8_UINT`). Each depth image uses device-local memory and is transitioned to `VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL`.

### Render pass

The render pass has two attachments:

| Attachment | Format | Load op | Store op | Initial layout | Final layout |
|-----------|--------|---------|---------|---------------|-------------|
| Colour | swap chain format | `CLEAR` | `STORE` | `UNDEFINED` | `PRESENT_SRC_KHR` |
| Depth | depth format | `CLEAR` | `DONT_CARE` | `UNDEFINED` | `DEPTH_STENCIL_ATTACHMENT_OPTIMAL` |

Clear values: colour → `{0, 0, 0, 1}` (black), depth → `{1.0, 0}`.

### Recreation on resize

`VlknRenderer::recreateSwapChain()` is called when:

- `vkAcquireNextImageKHR` returns `VK_ERROR_OUT_OF_DATE_KHR`, or
- `vkQueuePresentKHR` returns `VK_ERROR_OUT_OF_DATE_KHR` or `VK_SUBOPTIMAL_KHR`, or
- `VlknWindow::wasWindowResized()` returns `true` at the start of a frame.

The new `VlknSwapChain` is constructed with the old swap chain as a parameter (`std::shared_ptr<VlknSwapChain> previous`), which is passed to `vkCreateSwapchainKHR` as `oldSwapchain`. This allows the driver to reuse resources from the previous swap chain for a faster transition. After construction, the old swap chain is released. If the new and old swap chains have the same image format and depth format, the existing pipelines and render pass remain valid and do not need to be recreated.

---

## Synchronisation

vlkn uses a combination of semaphores and fences for GPU-CPU and GPU-GPU synchronisation.

### Per-frame-in-flight objects (2 frames)

| Object | Count | Purpose |
|--------|-------|---------|
| `imageAvailableSemaphores` | 2 | GPU: signal when `vkAcquireNextImageKHR` completes |
| `renderFinishedSemaphores` | 2 | GPU: signal when command buffer submission completes |
| `inFlightFences` | 2 | CPU: wait before reusing a frame's command buffer |

### Per-swap-chain-image objects

| Object | Count | Purpose |
|--------|-------|---------|
| `imagesInFlight` | swap chain image count | Pointer to the in-flight fence currently rendering to this image, preventing `vkQueueSubmit` to an image that the GPU is still presenting |

### Frame timeline

```
CPU                                GPU
│                                  │
│  vkWaitForFences(inFlightFence)  │  (wait for previous use of this
│  ◄────────────────────────────── │   frame slot to complete)
│                                  │
│  vkAcquireNextImageKHR           │
│  ──────────────────────────────► │  signal imageAvailableSemaphore
│  (returns imageIndex)            │   when image is available
│                                  │
│  if imagesInFlight[imageIndex]   │
│    vkWaitForFences(...)          │  (wait if a previous frame is
│                                  │   still rendering to this image)
│  vkResetFences(inFlightFence)    │
│                                  │
│  record commands into            │
│  commandBuffers[frameIndex]      │
│                                  │
│  vkQueueSubmit                   │
│    wait:   imageAvailableSemaphore
│    signal: renderFinishedSemaphore
│    fence:  inFlightFence         │
│  ──────────────────────────────► │  execute command buffer
│                                  │
│  vkQueuePresentKHR               │
│    wait: renderFinishedSemaphore │
│  ──────────────────────────────► │  present to display
│                                  │
```

### Shutdown

`vkDeviceWaitIdle()` is called before the `App` destructor tears down subsystems, ensuring all in-flight GPU work completes before any Vulkan resources are destroyed.

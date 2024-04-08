#pragma once

#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_pipeline.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

    class RenderSystem {
    public:
        RenderSystem(VlknDevice &device, VkRenderPass renderPass);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VlknGameObject> &gameObjects);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        VlknDevice &vlknDevice;
        std::unique_ptr<VlknPipeline> vlknPipeline;
        VkPipelineLayout pipelineLayout;
    };

} // namespace vlkn

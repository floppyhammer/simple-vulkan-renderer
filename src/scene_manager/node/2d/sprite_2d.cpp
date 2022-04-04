#include "sprite_2d.h"

#include "../sub_viewport.h"
#include "../../../rendering/mvp_buffer.h"
#include "../../../rendering/swap_chain.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Flint {
    Sprite2d::Sprite2d() {
        type = NodeType::Sprite2D;
    }

    void Sprite2d::update_mvp() {
        Node *viewport_node = get_viewport();

        Vec2<uint32_t> viewport_extent;
        if (viewport_node) {
            auto viewport = dynamic_cast<SubViewport *>(viewport_node);
            viewport_extent = viewport->get_extent();
        } else { // Default to swap chain image.
            auto extent = SwapChain::getSingleton().swapChainExtent;
            viewport_extent = Vec2<uint32_t>(extent.width, extent.height);
        }

        float sprite_width = material->texture->width * scale.x;
        float sprite_height = material->texture->height * scale.y;

        // Prepare UBO data. We use this matrix to convert a full-screen to the control's rect.
        UniformBufferObject ubo{};

        // The actual application order of these matrices is reverse.
        // 4.
        ubo.model = glm::translate(glm::mat4(1.0f),
                                   glm::vec3(position.x / viewport_extent.x * 2.0f,
                                             position.y / viewport_extent.y * 2.0f,
                                             0.0f));
        // 3.
        ubo.model = glm::translate(ubo.model, glm::vec3(-1.0, -1.0, 0.0f));
        // 2.
        ubo.model = glm::scale(ubo.model, glm::vec3(scale.x, scale.y, 1.0f));
        // 1.
        ubo.model = glm::scale(ubo.model,
                               glm::vec3(sprite_width / viewport_extent.x * 2.0f,
                                         sprite_height / viewport_extent.y * 2.0f,
                                         1.0f));

        mvp_buffer->update_uniform_buffer(ubo);
    }

    void Sprite2d::_draw(VkCommandBuffer p_command_buffer) {
        draw(p_command_buffer);

        Node::_draw(p_command_buffer);
    }

    void Sprite2d::draw(VkCommandBuffer p_command_buffer) {
        Node *viewport_node = get_viewport();

        VkPipeline pipeline = RS::getSingleton().blitGraphicsPipeline;

        if (viewport_node) {
            auto viewport = dynamic_cast<SubViewport *>(viewport_node);
            pipeline = viewport->viewport->blitGraphicsPipeline;
        }

        VkBuffer vertexBuffers[] = {mesh->vertexBuffer};
        RS::getSingleton().blit(
                p_command_buffer,
                pipeline,
                mesh->getDescriptorSet(SwapChain::getSingleton().currentImage),
                vertexBuffers,
                mesh->indexBuffer,
                mesh->indices_count);
    }
}

#include "control.h"

#include "../sub_viewport.h"
#include "../../rendering/swap_chain.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Flint {
    void Control::set_rect_position(float x, float y) {
        rect_position.x = x;
        rect_position.y = y;
    }

    void Control::set_rect_size(float w, float h) {
        rect_size.x = w;
        rect_size.y = h;
    }

    void Control::set_rect_scale(float x, float y) {
        rect_scale.x = x;
        rect_scale.y = y;
    }

    void Control::set_rect_rotation(float r) {
        rect_rotation = r;
    }

    void Control::set_rect_pivot_offset(float x, float y) {
        rect_pivot_offset.x = x;
        rect_pivot_offset.y = y;
    }

    Control::~Control() {
        auto device = Device::getSingleton().device;
        auto swapChainImages = SwapChain::getSingleton().swapChainImages;

        if (!vk_resources_allocated) return;

        // When we destroy the pool, the sets inside are destroyed as well.
        vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

        // Clean up uniform buffers.
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            vkDestroyBuffer(device, uniform_buffers[i], nullptr);
            vkFreeMemory(device, uniform_buffers_memory[i], nullptr);
        }

        // Clean up index buffer.
        vkDestroyBuffer(device, index_buffer, nullptr);
        vkFreeMemory(device, index_buffer_memory, nullptr);

        // Clean up vertex buffer.
        vkDestroyBuffer(device, vertex_buffer, nullptr); // GPU memory
        vkFreeMemory(device, vertex_buffer_memory, nullptr); // CPU memory
    }

    void Control::update(double delta) {
        // Branch to root.
        Node::update(delta);

        update_uniform_buffer();
    }

    void Control::update_uniform_buffer() {
        if (uniform_buffers_memory.empty()) return;

        Node *viewport_node = get_viewport();

        Vec2<uint32_t> viewport_extent;
        if (viewport_node) {
            auto viewport = dynamic_cast<SubViewport *>(viewport_node);
            viewport_extent = viewport->extent;
        } else { // Default to swap chain image.
            auto extent = SwapChain::getSingleton().swapChainExtent;
            viewport_extent = Vec2<uint32_t>(extent.width, extent.height);
        }

        // Prepare UBO data. We use this matrix to convert a full-screen to the control's rect.
        UniformBufferObject ubo{};

        // The actual application order of these matrices is reverse.
        // 4.
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(rect_position.x / viewport_extent.x * 2.0f,
                                                        rect_position.y / viewport_extent.y * 2.0f,
                                                        0.0f));
        // 3.
        ubo.model = glm::translate(ubo.model, glm::vec3(-1.0, -1.0, 0.0f));
        // 2.
        ubo.model = glm::scale(ubo.model, glm::vec3(rect_scale.x, rect_scale.y, 1.0f));
        // 1.
        ubo.model = glm::scale(ubo.model,
                               glm::vec3(rect_size.x / viewport_extent.x * 2.0f,
                                         rect_size.y / viewport_extent.y * 2.0f,
                                         1.0f));

        // Copy the UBO data to the current uniform buffer.
        RS::getSingleton().copyDataToMemory(&ubo.model,
                                            uniform_buffers_memory[SwapChain::getSingleton().currentImage],
                                            sizeof(ubo.model));
    }

    void Control::create_vertex_buffer() {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer; // In GPU
        VkDeviceMemory stagingBufferMemory; // In CPU

        // Create the GPU buffer and link it with the CPU memory.
        RS::getSingleton().createBuffer(bufferSize,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        stagingBuffer,
                                        stagingBufferMemory);

        // Copy data to the CPU memory.
        RS::getSingleton().copyDataToMemory((void *) vertices.data(), stagingBufferMemory, bufferSize);

        // Create the vertex buffer (GPU) and bind it to the vertex memory (CPU).
        RS::getSingleton().createBuffer(bufferSize,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        vertex_buffer,
                                        vertex_buffer_memory);

        // Copy buffer (GPU).
        RS::getSingleton().copyBuffer(stagingBuffer, vertex_buffer, bufferSize);

        // Clean up staging buffer and memory.
        vkDestroyBuffer(Device::getSingleton().device, stagingBuffer, nullptr);
        vkFreeMemory(Device::getSingleton().device, stagingBufferMemory, nullptr);
    }

    void Control::create_index_buffer() {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        RS::getSingleton().createBuffer(bufferSize,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        stagingBuffer,
                                        stagingBufferMemory);

        RS::getSingleton().copyDataToMemory((void *) indices.data(),
                                            stagingBufferMemory,
                                            bufferSize);

        RS::getSingleton().createBuffer(bufferSize,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        index_buffer,
                                        index_buffer_memory);

        // Copy data from staging buffer to index buffer.
        RS::getSingleton().copyBuffer(stagingBuffer, index_buffer, bufferSize);

        vkDestroyBuffer(Device::getSingleton().device, stagingBuffer, nullptr);
        vkFreeMemory(Device::getSingleton().device, stagingBufferMemory, nullptr);
    }

    void Control::create_uniform_buffers() {
        auto &swapChainImages = SwapChain::getSingleton().swapChainImages;

        VkDeviceSize bufferSize = sizeof(glm::mat4);

        uniform_buffers.resize(swapChainImages.size());
        uniform_buffers_memory.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            RS::getSingleton().createBuffer(bufferSize,
                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                            uniform_buffers[i],
                                            uniform_buffers_memory[i]);
        }
    }
}

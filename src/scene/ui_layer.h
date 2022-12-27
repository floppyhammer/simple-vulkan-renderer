#ifndef FLINT_UI_LAYER_H
#define FLINT_UI_LAYER_H

#include <memory>
#include <vector>

#include "node.h"
#include "resources/mesh.h"

namespace Flint {

/// UiLayer will draw every child UI nodes directly to the screen.
class UiLayer : public Node {
public:
    UiLayer();

    void draw(VkCommandBuffer cmd_buffer) override;

    void propagate_draw(VkCommandBuffer cmd_buffer) override;

    void update_mvp();

    std::shared_ptr<Mesh2d> mesh;

    Vec2I view_size;

    MvpPushConstant push_constant;
};

} // namespace Flint

#endif // FLINT_UI_LAYER_H
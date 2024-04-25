#ifndef FLINT_NODE_SUB_WINDOW_PROXY_H
#define FLINT_NODE_SUB_WINDOW_PROXY_H

#include "../common/geometry.h"
#include "node.h"

namespace Flint {

/// A sub-window besides the primary window.
class SubWindow : public Node {
    friend class SceneTree;

public:
    explicit SubWindow(Vec2I size);

    void update(double dt) override;

    void pre_draw_children() override;

    void post_draw_children() override;

    Vec2I get_size() const;

    void set_visibility(bool visible) override;

    std::shared_ptr<Pathfinder::Window> get_raw_window() const {
        return window_;
    }

    std::shared_ptr<Pathfinder::SwapChain> get_swap_chain() const {
        return swap_chain_;
    }

    std::shared_ptr<Pathfinder::Texture> get_vector_target() const {
        return vector_target_;
    }

    void set_vector_target(std::shared_ptr<Pathfinder::Texture> texture) {
        vector_target_ = texture;
    }

protected:
    Vec2I size_;

    std::shared_ptr<Pathfinder::Window> window_;
    std::shared_ptr<Pathfinder::SwapChain> swap_chain_;
    std::shared_ptr<Pathfinder::Texture> vector_target_;

    void record_commands() const;

private:
    struct {
        std::shared_ptr<Pathfinder::Scene> previous_scene;
    } temp_draw_data;

    float scale_factor = 1.0;
};

} // namespace Flint

#endif // FLINT_NODE_SUB_WINDOW_PROXY_H

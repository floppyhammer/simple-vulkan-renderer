#include "container.h"

namespace Flint {

Container::Container() {
    type = NodeType::Container;

    debug_size_box.border_color = ColorU::white();
}

void Container::adjust_layout() {
    auto max_size = size.max(minimum_size);
    for (auto &child : children) {
        if (child->is_ui_node()) {
            auto cast_child = dynamic_cast<NodeUi *>(child.get());
            cast_child->set_position({0, 0});
            auto child_min_size = cast_child->calc_minimum_size();
            max_size = max_size.max(child_min_size);
        }
    }

    size = max_size;

    for (auto &child : children) {
        if (child->is_ui_node()) {
            auto cast_child = dynamic_cast<NodeUi *>(child.get());
            cast_child->set_size(max_size);
        }
    }
}

Vec2F Container::calc_minimum_size() const {
    Vec2F child_min_size;
    for (auto &child : children) {
        if (child->is_ui_node()) {
            auto cast_child = dynamic_cast<NodeUi *>(child.get());
            auto min_size = cast_child->calc_minimum_size();
            child_min_size = child_min_size.max(min_size);
        }
    }

    return minimum_size.max(child_min_size);
}

void Container::set_size(Vec2F new_size) {
    if (size == new_size) {
        return;
    }

    auto min_size = calc_minimum_size();

    size = new_size.max(min_size);
}

void Container::update(double dt) {
    NodeUi::update(dt);

    adjust_layout();
}

} // namespace Flint

#include "node.h"

#include <string>

#include "render/swap_chain.h"
#include "sub_viewport.h"

namespace Flint {

void Node::propagate_update(double dt) {
    update(dt);

    for (auto &child : children) {
        child->propagate_update(dt);
    }
}

void Node::propagate_draw(VkCommandBuffer p_command_buffer) {
    draw(p_command_buffer);

    for (auto &child : children) {
        child->propagate_draw(p_command_buffer);
    }
}

void Node::propagate_notify(Signal signal) {
    notify(signal);

    for (auto &child : children) {
        child->propagate_notify(signal);
    }
}

void Node::propagate_input(InputEvent &event) {
    auto it = children.rbegin();
    while (it != children.rend()) {
        (*it)->propagate_input(event);
        it++;
    }

    input(event);
}

void Node::propagate_cleanup() {
    for (auto &child : children) {
        child->propagate_cleanup();
    }
}

void Node::input(InputEvent &event) {
}

void Node::update(double delta) {
}

void Node::notify(Signal signal) {
}

void Node::draw(VkCommandBuffer p_command_buffer) {
}

Node *Node::get_viewport() {
    if (parent == nullptr) {
        return nullptr;
    }

    return parent->type == NodeType::SubViewport ? parent : parent->get_viewport();
}

Vec2I Node::get_viewport_size() {
    Vec2I size;

    Node *viewport_node = get_viewport();

    if (viewport_node) {
        auto viewport = dynamic_cast<SubViewport *>(viewport_node);
        size = viewport->get_extent();
    } else { // Default to swap chain image.
        auto extent = SwapChain::getSingleton()->swapChainExtent;
        size = Vec2I(extent.width, extent.height);
    }

    return size;
}

void Node::set_parent(Node *node) {
    parent = node;
}

Node *Node::get_parent() const {
    return parent;
}

std::vector<std::shared_ptr<Node>> Node::get_children() {
    return children;
}

void Node::add_child(const std::shared_ptr<Node> &new_child) {
    // Set self as the parent of the new node.
    new_child->parent = this;

    //        if (p_child->name.empty()) {
    //            auto node_type_name = NodeTypeName[(uint32_t) p_child->type];
    //
    //            uint32_t same_type_child_count = 0;
    //            for (auto &c: children) {
    //                if (c->type == p_child->type) {
    //                    same_type_child_count++;
    //                }
    //            }
    //
    //            p_child->name = node_type_name + std::to_string(children.size());
    //        }

    children.push_back(new_child);

    //        get_tree()->
}

std::shared_ptr<Node> Node::get_child(size_t index) {
    if (index > children.size()) {
        return nullptr;
    }

    return children[index];
}

void Node::remove_child(size_t index) {
    if (index < 0 || index >= children.size()) return;
    children.erase(children.begin() + index);
}

bool Node::is_gui_node() const {
    return type >= NodeType::Control && type < NodeType::Node2D;
}

void Node::set_visibility(bool _visible) {
    visible = _visible;
}

bool Node::get_visibility() const {
    return visible;
}

NodeType Node::extended_from_which_base_node() const {
    if (type < NodeType::Control)
        return NodeType::Node;
    else if (type < NodeType::Node2D)
        return NodeType::Control;
    else if (type < NodeType::Node3D)
        return NodeType::Node2D;
    else if (type < NodeType::Max)
        return NodeType::Node3D;
    else
        return NodeType::Max;
}

std::string Node::get_node_path() const {
    auto type_name = NodeTypeName[static_cast<unsigned __int64>(type)];

    if (parent) {
        return parent->get_node_path() + "/" + type_name;
    } else {
        return "/" + type_name;
    }
}

void Node::when_subtree_changed() {
    for (auto &callback : subtree_changed_callbacks) {
        callback();
    }

    // Branch->root signal propagation.
    if (parent) {
        parent->when_subtree_changed();
    }
}

void Node::connect_signal(const std::string &signal, const std::function<void()> &callback) {
    if (signal == "subtree_changed") {
        subtree_changed_callbacks.push_back(callback);
    }
}

void Node::set_debug_mode(bool enabled) {
    debug_mode = enabled;
}

NodeType Node::get_node_type() const {
    return type;
}

} // namespace Flint
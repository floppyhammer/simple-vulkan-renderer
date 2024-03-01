#include "node.h"

#include <servers/render_server.h>

#include <string>

#include "sub_window.h"

namespace Flint {

void Node::propagate_update(double dt) {
    update(dt);

    for (auto &child : children) {
        child->propagate_update(dt);
    }
}

void Node::propagate_draw() {
    draw();

    for (auto &child : children) {
        child->propagate_draw();
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

void Node::draw() {
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
    new_child->tree_ = tree_;

    children.push_back(new_child);
}

std::shared_ptr<Node> Node::get_child(size_t index) {
    if (index > children.size()) {
        return nullptr;
    }

    return children[index];
}

void Node::remove_child(size_t index) {
    if (index < 0 || index >= children.size()) {
        return;
    }
    children.erase(children.begin() + index);
}

void Node::set_visibility(bool visible) {
    visible_ = visible;
}

bool Node::get_visibility() const {
    return visible_;
}

bool Node::get_global_visibility() const {
    if (parent) {
        return parent->get_global_visibility() && get_visibility();
    }

    return get_visibility();
}

Pathfinder::Window *Node::get_window() const {
    if (type == NodeType::Window) {
        auto sub_window_node = (SubWindow *)this;
        return sub_window_node->get_raw_window().get();
    }

    if (parent) {
        return parent->get_window();
    }

    return RenderServer::get_singleton()->window_builder_->get_primary_window().get();
}

std::string Node::get_node_path() const {
    std::string type_name = ""; // NodeTypeName[static_cast<unsigned __int64>(type)];

    if (parent) {
        return parent->get_node_path() + "/" + type_name;
    }

    return "/" + type_name;
}

void Node::when_parent_size_changed(Vec2F new_size) {
    for (auto &child : children) {
        child->when_parent_size_changed(new_size);
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

void Node::enable_visual_debug(bool enabled) {
    visual_debug = enabled;
}

NodeType Node::get_node_type() const {
    return type;
}

SceneTree *Node::get_tree() const {
    return tree_;
}

} // namespace Flint
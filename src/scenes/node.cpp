#include "node.h"

namespace Flint {
    void Node::update(double delta) {
        for (auto &child: children) {
            child->update(delta);
        }
    }

    void Node::draw() {
        for (auto &child: children) {
            child->draw();
        }

        Logger::verbose("DRAW", "Node");
    }

    void Node::notify(Signal signal) {
        for (auto &child: children) {
            notify(signal);
        }
    }

    Node *Node::get_viewport() {
        if (get_parent() != nullptr) {
            if (get_parent()->type == NodeType::SubViewport) {
                return get_parent();
            } else {
                return get_parent()->get_viewport();
            }
        } else {
            return {};
        }
    }

    Node *Node::get_parent() {
        return parent;
    }

    std::vector<std::shared_ptr<Node>> Node::get_children() {
        return children;
    }

    void Node::add_child(const std::shared_ptr<Node> &p_child) {
        // Set self as parent of the new node.
        p_child->parent = this;

        children.push_back(p_child);
    }

    void Node::remove_child(size_t index) {
        if (index < 0 || index >= children.size()) return;
        children.erase(children.begin() + index);
    }

    void Node::cleanup() {
        for (auto &child: children) {
            child->cleanup();
        }
    }
}
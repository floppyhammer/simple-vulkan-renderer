#include "node.h"

#include "graph.h"

namespace Flint::Ecs {

NodeId::NodeId() {
    v = uuids::uuid_system_generator{}();
    assert(!v.is_nil());
    assert(v.version() == uuids::uuid_version::random_number_based);
    assert(v.variant() == uuids::uuid_variant::rfc);
}

std::vector<SlotInfo> Node::input() const {
    return {};
}

std::vector<SlotInfo> Node::output() const {
    return {};
}

void Node::update() {
}

NodeRunError Node::run(const RenderGraphContext& graph, RenderContext& render_context) const {
    return NodeRunError::None;
}

} // namespace Flint::Ecs

#ifndef FLINT_ECS_RENDER_H
#define FLINT_ECS_RENDER_H

#include <optional>
#include <unordered_map>

#include "../common/mat3x3.h"
#include "../common/quat.h"
#include "data.h"

using namespace Flint::Math;

namespace Flint::Ecs {

struct C_Transform {
    Pathfinder::Vec3F translation;
    Quat rotation;
    Pathfinder::Vec3F scale;
};

struct C_GlobalTransform {
    Mat3x3<float> matrix3;
    Pathfinder::Vec3F translation;
};

struct C_Visibility {
    bool visible = true;
};

// Should be used with a Handle.
struct Mesh {
    float x;
};

// Should be used with a Handle.
struct Material {
    /// Returns this material's vertex shader. If [`ShaderRef::Default`] is returned, the default mesh vertex shader
    /// will be used.
    std::string vertex_shader() {
        return {};
    };

    /// Returns this material's fragment shader. If [`ShaderRef::Default`] is returned, the default mesh fragment shader
    /// will be used.
    std::string fragment_shader() {
        return {};
    };
};

/// A component bundle for entities with a [`Mesh`] and a [`Material`].
struct MaterialMeshBundle {
    C_Handle<Mesh> mesh;
    C_Handle<Material> material;
    C_Transform transform;
    C_GlobalTransform global_transform;
    /// User indication of whether an entity is visible
    C_Visibility visibility;
};

/// Stores all uniforms of the component type.
struct R_ComponentUniforms {
    std::unordered_map<uint64_t, std::shared_ptr<int>> uniforms;
};

/// Cache for all pipelines.
struct R_PipelineCache {
    std::unordered_map<uint64_t, std::shared_ptr<int>> pipelines;
};

} // namespace Flint::Ecs

#endif // FLINT_ECS_RENDER_H

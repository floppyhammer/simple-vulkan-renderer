#ifndef FLINT_IMAGE_H
#define FLINT_IMAGE_H

#include "../common/geometry.h"
#include "resource.h"

namespace Flint {

enum class ImageType {
    Raster,
    Vector,
    Render,
    Max,
};

class Image : public Resource {
public:
    Image(Vec2I size_) {
        size = size_;
    }

    explicit Image(const std::string &path);

    Vec2I get_size();

    ImageType get_type();

protected:
    ImageType type = ImageType::Max;

    Vec2I size;
};

} // namespace Flint

#endif // FLINT_IMAGE_H

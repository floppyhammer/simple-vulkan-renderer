#ifndef FLINT_PROGRESS_BAR_H
#define FLINT_PROGRESS_BAR_H

#include <functional>

#include "../../resources/style_box.h"
#include "control.h"
#include "label.h"

namespace Flint {
class ProgressBar : public Control {
public:
    ProgressBar();

    void update(double dt) override;

    void draw(VkCommandBuffer cmd_buffer) override;

    void set_position(Vec2F new_position) override;

    void set_size(Vec2F new_size) override;

    Vec2F calc_minimum_size() const override;

    void set_value(float new_value);

    void value_changed();

    void connect_signal(const std::string& signal, const std::function<void()>& callback);

protected:
    float value = 50, min_value = 0, max_value = 1000, step = 1;
    float ratio;

    std::optional<StyleBox> theme_progress, theme_bg, theme_fg;

    std::shared_ptr<Label> label;

    std::vector<std::function<void()>> on_value_changed;
};
} // namespace Flint

#endif // FLINT_PROGRESS_BAR_H

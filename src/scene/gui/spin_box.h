#ifndef FLINT_SPIN_BOX_H
#define FLINT_SPIN_BOX_H

#include <functional>

#include "button.h"
#include "control.h"
#include "label.h"
#include "resources/style_box.h"
#include "stack_container.h"
#include "texture_rect.h"

namespace Flint {
/**
 * Numerical input field.
 */
class SpinBox : public Control {
public:
    SpinBox();

    bool pressed = false;
    bool hovered = false;
    bool pressed_inside = false;
    bool drag_to_adjust_value = false;

    void input(InputEvent &event) override;

    void update(double dt) override;

    void draw(VkCommandBuffer p_command_buffer) override;

    void set_position(Vec2F p_position) override;

    void set_size(Vec2F p_size) override;

    Vec2F calc_minimum_size() const override;

    void connect_signal(std::string signal, std::function<void()> callback);

    void set_value(float p_value);

protected:
    float value = 0;

    /// Only for display, the real value will not be rounded.
    uint32_t rounding_digits = 3;

    bool is_integer = false;

    bool clamped = false;

    float min_value = 0;
    float max_value = 100;

    float step = 1.4;

    bool focused = false;

    std::shared_ptr<HStackContainer> container_h;
    std::shared_ptr<VStackContainer> container_v;
    std::shared_ptr<Button> increase_button, decrease_button;
    std::shared_ptr<Label> label;

    std::vector<std::function<void()>> on_focused_callbacks;

    std::optional<StyleBox> theme_normal, theme_focused;

protected:
    void on_focused();
};
} // namespace Flint

#endif // FLINT_SPIN_BOX_H
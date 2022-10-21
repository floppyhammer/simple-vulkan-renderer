#ifndef FLINT_LABEL_H
#define FLINT_LABEL_H

#include <cstdint>
#include <memory>

#include "../../../common/geometry.h"
#include "../../../resources/font.h"
#include "../../../resources/resource_manager.h"
#include "../../../resources/style_box.h"
#include "control.h"

using Pathfinder::Rect;

namespace Flint {

enum class Alignment {
    Begin,
    Center,
    End,
};

class Label : public Control {
public:
    Label(const std::string &p_text);

    /**
     * Set text context.
     * @note See https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html for glyph conventions.
     * @param p_text Text string.
     */
    void set_text(const std::string &p_text);

    std::string get_text() const;

    void insert_text(uint32_t position, const std::string &p_text);

    void remove_text(uint32_t position, uint32_t count);

    void set_size(Vec2<float> p_size) override;

    void set_font(std::shared_ptr<Font> p_font);

    void set_text_style(float p_size, ColorU p_color, float p_stroke_width, ColorU p_stroke_color);

    void update(double dt) override;

    void draw(VkCommandBuffer p_command_buffer) override;

    void set_horizontal_alignment(Alignment alignment);

    void set_vertical_alignment(Alignment alignment);

    Vec2<float> calculate_minimum_size() const override;

    std::vector<Glyph> &get_glyphs();

    float get_font_size() const;

private:
    void measure();

    void consider_alignment();

    Vec2<float> get_text_size() const;

private:
    std::wstring text;

    std::shared_ptr<Font> font;

    bool clip = false;

    float font_size = 32;

    std::vector<Glyph> glyphs;

    mutable Rect<float> layout_box;

    FontStyle font_style;
    // Fill
    ColorU color{163, 163, 163, 255};

    // Stroke
    float stroke_width = 0;
    ColorU stroke_color;

    // Layout
    Alignment horizontal_alignment = Alignment::Begin;
    Alignment vertical_alignment = Alignment::Begin;
    Vec2<float> alignment_shift{0};

    std::optional<StyleBox> theme_background;
};
} // namespace Flint

#endif // FLINT_LABEL_H

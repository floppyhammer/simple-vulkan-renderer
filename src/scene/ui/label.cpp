#include "label.h"

#include <string>

using Pathfinder::Transform2;

namespace Flint {

enum class Bidi {
    Auto,
    LeftToRight,
    RightToLeft,
};

Label::Label(const std::string &_text) {
    type = NodeType::Label;

    debug_size_box.border_color = ColorU::red();

    font = ResourceManager::get_singleton()->load<Font>("../assets/unifont-14.0.03.ttf");

    set_text(_text);

    font_style.color = {163, 163, 163, 255};
}

void Label::set_text(const std::string &new_text) {
    // Only update glyphs when text has changed.
    if (text == new_text || font == nullptr) {
        return;
    }

    text = new_text;

    text_debug = utf8_to_ws(new_text);

    measure();
}

void Label::insert_text(uint32_t position, const std::string &new_text) {
    if (new_text.empty()) {
        return;
    }

    text.insert(position, new_text);

    measure();
}

void Label::remove_text(uint32_t position, uint32_t count) {
    if (position == -1) return;

    text.erase(position, count);

    measure();
}

std::string Label::get_text() const {
    return text;
}

void Label::set_size(Vec2F p_size) {
    if (size == p_size) return;

    size = p_size;
    consider_alignment();
}

void Label::measure() {
    // Get font info.
    int ascent = font->get_ascent();
    int descent = font->get_descent();

    font->get_glyphs_harfbuzz(text, language, glyphs);

    // Reset text's layout box.
    layout_box = RectF();

    for (auto &g : glyphs) {
        // Set UTF-32 codepoint.
        //        g.text = u_codepoint;

        // Set glyph index.
        //        g.index = font->find_index(u_codepoint);

        // Baseline offset.
        //        g.x_off = x;
        //        g.y_off = y;

        // Line break.
        //        if (u_codepoint == '\n') {
        //            x = 0;
        //            y += font_size;
        //            glyphs.push_back(g);
        //            continue;
        //        }

        // Glyph width.
        //        g.advance = font->get_advance(g.index);

        // Get the glyph path's bounding box. The Y axis points down.
        RectI bounding_box = font->get_bounds(g.index);

        // Set glyph path.
        g.path = font->get_glyph_path(g.index);

        // The position of the left point of the glyph's baseline in the whole text.
        // g.position = Vec2F(g.x_off, g.y_off);

        // Move the center to the top-left corner of the glyph's layout box.
        // g.position.y += ascent;

        // The glyph's layout box in the glyph's local coordinates. The origin is the baseline.
        // The Y axis is downward.
        g.box = RectF(0, -ascent, g.x_advance, -descent);

        // BBox in the glyph's local coordinates.
        g.bbox = bounding_box.to_f32();

        // The glyph's layout box in the text's local coordinates. The origin is the top-left corner of the text box.
        g.layout_box = RectF(g.x_offset, g.y_offset, g.x_offset + g.x_advance, g.y_offset + font_size);

        // The whole text's layout box.
        layout_box = layout_box.union_rect(g.layout_box);

        // Advance x.
        // x += roundf(g.advance);
    }
}

void Label::set_font(std::shared_ptr<Font> p_font) {
    if (p_font == nullptr) {
        return;
    }

    font = std::move(p_font);

    if (text.empty()) {
        return;
    }

    measure();
}

void Label::consider_alignment() {
    alignment_shift = Vec2F(0);

    switch (horizontal_alignment) {
        case Alignment::Begin:
            break;
        case Alignment::Center: {
            alignment_shift.x = size.x * 0.5f - layout_box.center().x;
        } break;
        case Alignment::End: {
            alignment_shift.x = size.x - layout_box.width();
        } break;
    }

    switch (vertical_alignment) {
        case Alignment::Begin:
            break;
        case Alignment::Center: {
            alignment_shift.y = size.y * 0.5f - layout_box.center().y;
        } break;
        case Alignment::End: {
            alignment_shift.y = size.y - layout_box.height();
        } break;
    }
}

void Label::update(double dt) {
}

void Label::set_text_style(float p_size, ColorU p_color, float p_stroke_width, ColorU p_stroke_color) {
    font_size = p_size;
    color = p_color;
    stroke_width = p_stroke_width;
    stroke_color = p_stroke_color;

    measure();
}

void Label::draw() {
    if (!visible) {
        return;
    }

    auto global_position = get_global_position();

    auto vector_server = VectorServer::get_singleton();

    if (theme_background.has_value()) {
        vector_server->draw_style_box(theme_background.value(), global_position, size);
    }

    auto translation = global_position + alignment_shift + Vec2F(0, 1) * font->get_ascent();

    RectF clip_box;
    //    if (clip) {
    //        clip_box = {{}, size};
    //    } else {
    //        clip_box = {{}, calc_minimum_size()};
    //    }

    vector_server->draw_glyphs(glyphs, font_style, Transform2::from_translation(translation), clip_box);

    NodeUi::draw();
}

void Label::set_horizontal_alignment(Alignment alignment) {
    if (horizontal_alignment == alignment) return;

    horizontal_alignment = alignment;

    consider_alignment();
}

void Label::set_vertical_alignment(Alignment alignment) {
    if (vertical_alignment == alignment) return;

    vertical_alignment = alignment;

    consider_alignment();
}

Vec2<float> Label::calc_minimum_size() const {
    auto min_size = get_text_size();
    min_size.y = font_size;

    return min_size.max(minimum_size);
}

Vec2<float> Label::get_text_size() const {
    return layout_box.is_valid() ? layout_box.size() : Vec2<float>(0);
}

std::vector<Glyph> &Label::get_glyphs() {
    return glyphs;
}

float Label::get_font_size() const {
    return font_size;
}

void Label::set_language(Language new_lang) {
    if (language == new_lang) {
        return;
    }

    language = new_lang;

    measure();
}

} // namespace Flint

#include "label.h"

#include <list>
#include <string>

#include "../../resources/default_resource.h"

using Pathfinder::Transform2;

namespace Flint {

enum class Bidi {
    Auto,
    LeftToRight,
    RightToLeft,
};

std::vector<Pathfinder::Range> get_line_breakable_groups(const std::vector<InContextGlyph> &glyphs, int offset) {
    std::vector<Pathfinder::Range> groups;

    bool rtl = false;

    if (rtl) {
        //        uint32_t group_start = glyphs.size() - 1;
        //
        //        for (int g_idx = glyphs.size() - 1; g_idx >= 0; g_idx--) {
        //            auto &g = glyphs[g_idx];
        //
        //            if (g.line_breakable_ && g_idx != glyphs.size() - 1) {
        //                Pathfinder::Range group = {group_start, group_start - g_idx};
        //                group_start = g_idx;
        //                groups.push_back(group);
        //            }
        //        }
        //
        //        Pathfinder::Range group = {group_start, group_start + 1};
        //        groups.push_back(group);
        //
        //        for (auto &r : groups) {
        //            r.begin = r.begin + 1 - r.length;
        //        }
    } else {
        uint32_t group_start = 0;

        for (int g_idx = 0; g_idx < glyphs.size(); g_idx++) {
            auto &g = glyphs[g_idx];

            if (g.line_breakable_ && g_idx != 0) {
                Pathfinder::Range group = {offset + group_start, offset + static_cast<unsigned long long>(g_idx)};
                group_start = g_idx;
                groups.push_back(group);
            }
        }

        Pathfinder::Range group = {offset + group_start, offset + (uint32_t)glyphs.size()};
        groups.push_back(group);
    }

    return groups;
}

/// PARAs -> LINEs
std::vector<Line> get_lines_with_word_wrap(float limited_width,
                                           const std::vector<Line> &original_paras,
                                           const std::vector<InContextGlyph> &glyphs,
                                           Vec2F &out_text_size) {
    float tracking = 0;

    std::vector<Line> wrapped_lines;

    Vec2F text_size{};

    for (const auto &para : original_paras) {
        const auto &para_range = para.glyph_ranges;

        // Get glyphs in this paragraph.
        std::vector<InContextGlyph> para_glyphs;
        for (int glyph_idx = para_range.start; glyph_idx < para_range.end; glyph_idx++) {
            para_glyphs.push_back(glyphs[glyph_idx]);
        }

        // Get line-breakable groups in this paragraph.
        auto groups_in_para = get_line_breakable_groups(para_glyphs, para_range.start);

        std::vector<float> group_widths_in_para;

        // For RTL paragraphs, we handle the groups reversely.
        if (para.rtl) {
            std::reverse(groups_in_para.begin(), groups_in_para.end());
        }

        // Break groups that are too long.
        for (auto p_group = groups_in_para.begin(); p_group != groups_in_para.end(); p_group++) {
            auto group = *p_group;

            float group_width = 0;

            for (int j = para.rtl ? p_group->length() - 1 : 0; para.rtl ? j >= 0 : j < p_group->length();
                 para.rtl ? j-- : j++) {
                int glyph_idx = p_group->start + j;

                const Glyph &glyph = glyphs[glyph_idx].glyph_;
                float glyph_width = glyph.x_advance;

                // Handle some abonormal graphs which are too wide.
                if (group_width == 0 && glyph_width > limited_width) {
                    break;
                }

                // Break the current group if it is too wide.
                if ((group_width + glyph_width + tracking) > limited_width) {
                    // Break the current group into two ones.
                    Pathfinder::Range range_pre;
                    Pathfinder::Range range_next;

                    if (para.rtl) {
                        range_pre = {p_group->start + j + 1, p_group->start + p_group->length()};
                        range_next = {p_group->start, p_group->start + j + 1};
                    } else {
                        range_pre = {p_group->start, p_group->start + j};
                        range_next = {p_group->start + j, p_group->start + p_group->length()};
                    }

                    // Replace the old group with the two newly created groups.
                    *p_group = range_next;
                    p_group = groups_in_para.insert(p_group, range_pre);

                    // Move on to handle the next group.
                    break;
                }

                group_width += glyph_width;
                group_width += tracking;
            }

            group_width -= tracking;
            group_widths_in_para.push_back(group_width);
        }

        // Up to this point, all the groups in the paragraph meet the width requirement.
        // We can start breaking the paragraph into lines.

        int current_group_idx = 0;
        float current_line_width = 0;
        std::vector<Pathfinder::Range> current_line_groups;

        while (!groups_in_para.empty()) {
            Pathfinder::Range current_group;

            current_group = groups_in_para.front();
            groups_in_para.erase(groups_in_para.begin());

            float current_group_width = group_widths_in_para[current_group_idx];

            // Handle some abonormal graphs which are too wide.
            if (current_line_groups.empty() && current_group_width > limited_width) {
                Pathfinder::Range new_range = {current_group.start, current_group.end};
                wrapped_lines.push_back({new_range, para.rtl, current_group_width});

                text_size.x = std::max(current_group_width, text_size.x);
                current_group_idx++;
                continue;
            }

            // Finish a line.
            if (current_line_width + current_group_width + tracking > limited_width) {
                uint32_t line_start = current_line_groups.front().start;
                uint32_t line_end = current_line_groups.front().end;

                for (auto &group : current_line_groups) {
                    line_start = std::min(line_start, (uint32_t)group.start);
                    line_end = std::max(line_end, (uint32_t)group.end);
                }

                Pathfinder::Range new_range = {line_start, line_end};
                wrapped_lines.push_back({new_range, para.rtl, current_line_width});

                text_size.x = std::max(current_line_width, text_size.x);

                current_line_groups.clear();
                current_line_width = 0;
            }

            current_line_groups.push_back(current_group);
            current_line_width += current_group_width + tracking;
            current_group_idx++;
        }

        if (!current_line_groups.empty()) {
            uint32_t line_start = current_line_groups.front().start;
            uint32_t line_end = current_line_groups.front().end;

            for (auto &group : current_line_groups) {
                line_start = std::min(line_start, (uint32_t)group.start);
                line_end = std::max(line_end, (uint32_t)group.end);
            }

            Pathfinder::Range new_range = {line_start, line_end};
            wrapped_lines.push_back({new_range, para.rtl, current_line_width});

            text_size.x = std::max(current_line_width, text_size.x);
        }
    }

    out_text_size = text_size;

    return wrapped_lines;
}

Label::Label() {
    type = NodeType::Label;

    debug_size_box.border_color = ColorU::blue();

    text_ = "Label";

    font = ResourceManager::get_singleton()->load<Font>("../assets/fonts/Arial Unicode MS Font.ttf");
    emoji_font = ResourceManager::get_singleton()->load<Font>("../assets/fonts/EmojiOneColor.otf");

    text_style.color = {163, 163, 163, 255};

    theme_background = DefaultResource::get_singleton()->get_default_theme()->label.styles["background"];
}

void Label::set_text(const std::string &new_text) {
    // Only update glyphs when text has changed.
    if (text_ == new_text || font == nullptr) {
        return;
    }

    text_ = new_text;
    utf8_to_utf32(text_, text_u32_);

    need_to_remeasure = true;
}

void Label::insert_text(uint32_t codepint_position, const std::string &new_text) {
    if (new_text.empty()) {
        return;
    }

    assert(codepint_position <= text_u32_.size() && "Codepoint index is out of bounds!");

    std::u32string new_text_u32;
    utf8_to_utf32(new_text, new_text_u32);

    text_u32_.insert(codepint_position, new_text_u32);
    text_ = utf32_to_utf8(text_u32_);

    need_to_remeasure = true;
}

void Label::remove_text(uint32_t codepint_position, uint32_t count) {
    assert((codepint_position + count) <= text_u32_.size() && "Codepoint index is out of bounds!");

    text_u32_.erase(codepint_position, count);
    text_ = utf32_to_utf8(text_u32_);

    need_to_remeasure = true;
}

std::string Label::get_sub_text(uint32_t codepint_position, uint32_t count) const {
    assert((codepint_position + count) <= text_u32_.size() && "Codepoint index is out of bounds!");

    auto subtext_u32 = text_u32_.substr(codepint_position, count);
    auto subtext = utf32_to_utf8(subtext_u32);

    return subtext;
}

std::string Label::get_text() const {
    return text_;
}

std::u32string Label::get_text_u32() const {
    return text_u32_;
}

void Label::set_size(Vec2F new_size) {
    if (size == new_size) {
        return;
    }

    layout_is_dirty = true;

    size = new_size.max(get_effective_minimum_size());
}

/// A very crude way for line-breaking.
std::vector<InContextGlyph> convert_to_in_context_glyphs(const std::vector<Glyph> &glyphs) {
    std::vector<InContextGlyph> in_context_glyphs;
    in_context_glyphs.reserve(glyphs.size());

    // Add emoji data.
    for (auto &glyph : glyphs) {
        InContextGlyph in_context_glyph;
        in_context_glyph.glyph_ = glyph;
        in_context_glyph.line_breakable_ = false;

        if (glyph.script == Script::Cjk) {
            in_context_glyph.line_breakable_ = true;
        } else {
            if (glyph.text == " ") {
                in_context_glyph.line_breakable_ = true;
            }
        }

        in_context_glyphs.push_back(in_context_glyph);
    }

    return in_context_glyphs;
}

void Label::measure() {
    // Get font info.
    int ascent = font->get_ascent();
    int descent = font->get_descent();

    font->get_glyphs(text_, glyphs_, para_ranges_);

    // Add emoji data.
    for (auto &glyph : glyphs_) {
        if (glyph.codepoints.size() == 1) {
            uint16_t glyph_index = emoji_font->find_glyph_index_by_codepoint(glyph.codepoints.front());
            glyph.svg = emoji_font->get_glyph_svg(glyph_index);
            if (!glyph.svg.empty() && glyph.index == 0) {
                glyph.x_advance = font->get_size();
                glyph.box = {0, 0, (float)font->get_size(), (float)font->get_size()};
                glyph.emoji = true;
            }
        }
    }

    in_context_glyphs_ = convert_to_in_context_glyphs(glyphs_);
}

void Label::make_layout() {
    // Reset text's layout box.
    layout_box = RectF();

    glyph_positions.clear();

    glyph_boxes.clear();
    character_boxes.clear();

    float line_height = font->get_size();

    float cursor_x = 0;
    float cursor_y = 0;

    if (word_wrap_) {
        Vec2F text_size{};
        line_ranges_ = get_lines_with_word_wrap(size.x, para_ranges_, in_context_glyphs_, text_size);
    }

    const auto &effective_line_ranges = word_wrap_ ? line_ranges_ : para_ranges_;

    float effective_max_line_width = 0;
    if (word_wrap_) {
        for (const auto &line : effective_line_ranges) {
            effective_max_line_width = std::max(effective_max_line_width, line.width);
        }
    } else {
        effective_max_line_width = size.x;
    }

    glyph_positions.resize(glyphs_.size());

    // Build layout.
    for (const auto &line : effective_line_ranges) {
        const auto &range = line.glyph_ranges;

        switch (bidi_alignment_) {
            case BidiAlignment::Auto: {
                if (line.rtl) {
                    cursor_x = effective_max_line_width - line.width;
                }
            } break;
            case BidiAlignment::Begin: {
            } break;
            case BidiAlignment::Center: {
                cursor_x = effective_max_line_width * 0.5f - line.width * 0.5f;
            } break;
            case BidiAlignment::End: {
                cursor_x = effective_max_line_width - line.width;
            } break;
        }

        for (int i = range.start; i < range.end; i++) {
            const auto &g = glyphs_[i];

            // The glyph's layout box in the text's local coordinates.
            // The origin is the top-left corner of the text box.
            RectF glyph_layout_box =
                RectF(cursor_x + g.x_offset, cursor_y + g.y_offset, cursor_x + g.x_advance, cursor_y + line_height);

            glyph_positions[i] = {cursor_x + g.x_offset, cursor_y + g.y_offset};

            // The whole text's layout box.
            layout_box = layout_box.union_rect(glyph_layout_box);

            // Advance x.
            cursor_x += g.x_advance;
        }

        cursor_x = 0;
        cursor_y += line_height;
    }
}

void Label::set_font(std::shared_ptr<Font> new_font) {
    if (new_font == nullptr) {
        return;
    }

    font = std::move(new_font);

    need_to_remeasure = true;
}

void Label::consider_alignment() {
    alignment_shift = Vec2F(0);

    // For multi line label, the text box always occupies the whole label area.
    if (multi_line_) {
        return;
    }

    // Make sure size has the correct value before using it.
    size = size.max(get_effective_minimum_size());

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
    NodeUi::update(dt);

    if (need_to_remeasure) {
        need_to_remeasure = false;
        measure();
    }
    if (layout_is_dirty) {
        layout_is_dirty = false;
        make_layout();
    }

    consider_alignment();
}

void Label::set_text_style(TextStyle _text_style) {
    text_style = _text_style;
}

void Label::draw() {
    if (!visible_) {
        return;
    }

    auto global_position = get_global_position();

    auto vector_server = VectorServer::get_singleton();

    vector_server->draw_style_box(theme_background, global_position, size);

    auto baseline_position = Vec2F(0, font->get_ascent());

    auto translation = Transform2::from_translation(global_position + alignment_shift + baseline_position);

    RectF clip_box;
    //    if (clip) {
    //        clip_box = {{}, size};
    //    } else {
    //        clip_box = {{}, calc_minimum_size()};
    //    }

    vector_server->draw_glyphs(glyphs_, glyph_positions, text_style, translation, clip_box);

    NodeUi::draw();
}

void Label::set_horizontal_alignment(Alignment alignment) {
    if (horizontal_alignment == alignment) {
        return;
    }

    horizontal_alignment = alignment;
}

void Label::set_vertical_alignment(Alignment alignment) {
    if (vertical_alignment == alignment) {
        return;
    }

    vertical_alignment = alignment;
}

void Label::calc_minimum_size() {
    auto min_size = get_text_minimum_size();

    // Label has a minimal height even when the text is empty.
    min_size.y = std::max(min_size.y, (float)font->get_size());

    calculated_minimum_size = min_size;
}

Vec2F Label::get_text_minimum_size() const {
    if (word_wrap_) {
        return Vec2F(0);
    }

    float effective_max_para_width = 0;
    for (const auto &para : para_ranges_) {
        effective_max_para_width = std::max(effective_max_para_width, para.width);
    }

    return {effective_max_para_width, para_ranges_.size() * ((float)font->get_ascent() + (float)font->get_descent())};
}

std::vector<Glyph> &Label::get_glyphs() {
    return glyphs_;
}

std::shared_ptr<Font> Label::get_font() const {
    return font;
}

float Label::get_glyph_right_edge_position(int32_t glyph_index) {
    assert(glyph_index >= 0 && "Invalid glyph index!");

    float pos = 0;

    assert(glyph_index < glyphs_.size() && "Out of bounds glyph index!");

    for (int i = 0; i <= glyph_index; i++) {
        pos += glyphs_[i].x_advance;
    }

    return pos;
}

float Label::get_glyph_left_edge_position(int32_t glyph_index) {
    assert(glyph_index >= 0 && "Invalid glyph index!");

    float pos = 0;

    for (int i = 0; i < glyph_index; i++) {
        pos += glyphs_[i].x_advance;
    }

    return pos;
}

float Label::get_codepoint_left_edge_position(int32_t codepoint_index) {
    assert(codepoint_index >= 0 && "Invalid codepoint index!");

    float pos = 0;

    int32_t current_codepoint = 0;

    for (int i = 0; i < glyphs_.size(); i++) {
        if (current_codepoint == codepoint_index) {
            return pos;
        }
        const auto &glyph = glyphs_[i];
        int32_t glyph_codepoint_count = glyph.codepoints.size();

        for (int j = 0; j < glyph_codepoint_count; j++) {
            pos += glyph.x_advance / glyph_codepoint_count;

            current_codepoint++;
        }
    }

    return 0;
}

float Label::get_codepoint_right_edge_position(int32_t codepoint_index) {
    assert(codepoint_index >= 0 && "Invalid codepoint index!");

    float pos = 0;

    int32_t current_codepoint = 0;

    for (int i = 0; i < glyphs_.size(); i++) {
        const auto &glyph = glyphs_[i];
        int32_t glyph_codepoint_count = glyph.codepoints.size();

        for (int j = 0; j < glyph_codepoint_count; j++) {
            pos += glyph.x_advance / (float)glyph_codepoint_count;

            if (current_codepoint == codepoint_index) {
                return pos;
            }

            current_codepoint++;
        }
    }

    return 0;
}

float Label::get_position_by_codepoint(uint32_t codepoint_index) {
    float pos = 0;

    // TODO

    return pos;
}

uint32_t Label::get_codepoint_by_position(Vec2F position) {
    return 0;
}

} // namespace Flint

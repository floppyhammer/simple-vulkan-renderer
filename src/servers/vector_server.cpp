#include "vector_server.h"

using namespace Pathfinder;

namespace Flint {

void VectorServer::init(const std::shared_ptr<Pathfinder::Device> &device) {
    canvas = std::make_shared<Pathfinder::Canvas>(device);
}

void VectorServer::cleanup() {
    canvas.reset();
}

void VectorServer::set_dst_texture(const std::shared_ptr<ImageTexture> &texture) {
    auto pathfinder_texture =
        Pathfinder::TextureVk::from_wrapping({texture->get_size(), Pathfinder::TextureFormat::Rgba8Unorm},
                                             texture->image,
                                             texture->imageMemory,
                                             texture->imageView,
                                             texture->sampler,
                                             Pathfinder::TextureLayout::ShaderReadOnly);

    canvas->set_dst_texture(pathfinder_texture);
}

void VectorServer::submit_and_clear() {
    canvas->draw();
    canvas->clear();

    // TODO: clear the dst texture every frame even when there's nothing to draw on the canvas.
    // Get the dst texture.
    auto texture_vk = static_cast<Pathfinder::TextureVk *>(canvas->get_dst_texture().get());

    auto cmd_buffer = RenderServer::get_singleton()->beginSingleTimeCommands();

    // Transition the dst texture to ShaderReadOnly layout, so we can use it as a sampler.
    // Its layout may be Undefined or ColorAttachment.
    RenderServer::transitionImageLayout(cmd_buffer,
                                        texture_vk->get_image(),
                                        Pathfinder::to_vk_texture_format(texture_vk->get_format()),
                                        Pathfinder::to_vk_layout(texture_vk->get_layout()),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        1,
                                        1);
    texture_vk->set_layout(Pathfinder::TextureLayout::ShaderReadOnly);

    RenderServer::get_singleton()->endSingleTimeCommands(cmd_buffer);
}

std::shared_ptr<Pathfinder::Canvas> VectorServer::get_canvas() const {
    return canvas;
}

void VectorServer::draw_line(Vec2F start, Vec2F end, float width, ColorU color) {
    canvas->save_state();

    Pathfinder::Path2d path;
    path.add_line({start.x, start.y}, {end.x, end.y});

    canvas->set_stroke_paint(Pathfinder::Paint::from_color(color));
    canvas->set_line_width(width);
    // canvas->set_line_cap(Pathfinder::LineCap::Round);
    canvas->stroke_path(path);

    canvas->restore_state();
}

void VectorServer::draw_rectangle(const RectF &rect, float line_width, ColorU color) {
    canvas->save_state();

    Pathfinder::Path2d path;
    path.add_rect(rect);

    canvas->set_stroke_paint(Pathfinder::Paint::from_color(color));
    canvas->set_line_width(line_width);
    canvas->stroke_path(path);

    canvas->restore_state();
}

void VectorServer::draw_circle(Vec2F center, float radius, float line_width, bool fill, ColorU color) {
    canvas->save_state();

    Pathfinder::Path2d path;
    path.add_circle({center.x, center.y}, radius);

    if (fill) {
        canvas->set_fill_paint(Pathfinder::Paint::from_color(color));
        canvas->fill_path(path, Pathfinder::FillRule::Winding);
    } else if (line_width > 0) {
        canvas->set_stroke_paint(Pathfinder::Paint::from_color(color));
        canvas->set_line_width(line_width);
        canvas->stroke_path(path);
    }

    canvas->restore_state();
}

void VectorServer::draw_path(VectorPath &vector_path, Transform2 transform) {
    canvas->save_state();

    canvas->set_transform(global_transform_offset * transform);

    if (vector_path.fill_color.is_opaque()) {
        canvas->set_fill_paint(Pathfinder::Paint::from_color(vector_path.fill_color));
        canvas->fill_path(vector_path.path2d, Pathfinder::FillRule::Winding);
    }

    if (vector_path.stroke_width > 0) {
        canvas->set_stroke_paint(Pathfinder::Paint::from_color(vector_path.stroke_color));
        canvas->set_line_width(vector_path.stroke_width);
        canvas->stroke_path(vector_path.path2d);
    }

    canvas->restore_state();
}

void VectorServer::draw_image_texture(ImageTexture &texture, Transform2 transform) {
    canvas->save_state();

    canvas->set_transform(global_transform_offset * transform);

    auto image = texture.image_data;

    canvas->draw_image(image, RectF({}, Vec2F() + image->size.to_f32()));

    canvas->restore_state();
}

void VectorServer::draw_vector_texture(VectorTexture &texture, Transform2 transform) {
    for (auto &path : texture.get_paths()) {
        draw_path(path, transform);
    }

    if (texture.get_svg_scene()) {
        canvas->get_scene()->append_scene(*texture.get_svg_scene()->get_scene(), global_transform_offset * transform);
    }
}

void VectorServer::draw_style_box(const StyleBox &style_box, const Vec2F &position, const Vec2F &size) {
    auto path = Pathfinder::Path2d();
    path.add_rect({{}, size}, style_box.corner_radius);

    canvas->save_state();

    canvas->set_shadow_color(style_box.shadow_color);
    canvas->set_shadow_blur(style_box.shadow_size);

    auto transform = Pathfinder::Transform2::from_translation(position);
    canvas->set_transform(global_transform_offset * transform);

    canvas->set_fill_paint(Pathfinder::Paint::from_color(style_box.bg_color));
    canvas->fill_path(path, Pathfinder::FillRule::Winding);

    if (style_box.border_width > 0) {
        canvas->set_stroke_paint(Pathfinder::Paint::from_color(style_box.border_color));
        canvas->set_line_width(style_box.border_width);
        canvas->stroke_path(path);
    }

    canvas->restore_state();
}

void VectorServer::draw_style_line(const StyleLine &style_line, const Vec2F &start, const Vec2F &end) {
    auto path = Pathfinder::Path2d();
    path.add_line(start, end);

    canvas->save_state();

    canvas->set_transform(global_transform_offset);
    canvas->set_stroke_paint(Pathfinder::Paint::from_color(style_line.color));
    canvas->set_line_width(style_line.width);
    canvas->stroke_path(path);

    canvas->restore_state();
}

void VectorServer::draw_glyphs(std::vector<Glyph> &glyphs,
                               std::vector<Vec2F> &glyph_positions,
                               TextStyle text_style,
                               const Transform2 &transform,
                               const RectF &clip_box) {
    if (glyphs.size() != glyph_positions.size()) {
        Logger::error("Glyph count mismatches glyph position count!", "VectorServer");
        return;
    }

    canvas->save_state();

    // Text clip.
    if (clip_box.is_valid()) {
        auto clip_path = Path2d();
        clip_path.add_rect(clip_box, 0);
        canvas->set_transform(global_transform_offset * transform);
        canvas->clip_path(clip_path, FillRule::Winding);
    }

    auto skew_xform = Transform2::from_scale({1, 1});
    if (text_style.italic) {
        skew_xform = Transform2({1, 0, tan(-15.f * 3.1415926f / 180.f), 1}, {});
    }

    // Draw glyph strokes. The strokes go below the fills.
    for (int i = 0; i < glyphs.size(); i++) {
        auto &g = glyphs[i];
        auto &p = glyph_positions[i];

        if (g.emoji) {
            continue;
        }

        canvas->set_transform(global_transform_offset * Transform2::from_translation(p) * transform * skew_xform);

        // Add stroke if needed.
        canvas->set_stroke_paint(Paint::from_color(text_style.stroke_color));
        canvas->set_line_width(text_style.stroke_width);
        canvas->set_line_join(LineJoin::Round);
        canvas->stroke_path(g.path);
    }

    // Draw glyph fills.
    for (int i = 0; i < glyphs.size(); i++) {
        auto &g = glyphs[i];
        auto &p = glyph_positions[i];

        // No italic for emojis and debug boxes.
        auto glyph_global_transform = global_transform_offset * Transform2::from_translation(p) * transform;

        if (!g.emoji) {
            canvas->set_transform(glyph_global_transform * skew_xform);

            // Add fill.
            canvas->set_fill_paint(Paint::from_color(text_style.color));
            canvas->fill_path(g.path, FillRule::Winding);

            // Use stroke to make a pseudo bold effect.
            if (text_style.bold) {
                canvas->set_stroke_paint(Paint::from_color(text_style.color));
                canvas->set_line_width(1);
                canvas->set_line_join(LineJoin::Bevel);
                canvas->stroke_path(g.path);
            }
        } else {
            auto svg_scene = std::make_shared<Pathfinder::SvgScene>();
            svg_scene->load_from_string(g.svg, *canvas);

            // The emoji's svg size is always fixed for a specific font no matter what the font size you set.
            auto svg_size = svg_scene->get_scene()->get_view_box().size();
            auto glyph_size = g.box.size();

            auto emoji_scale = Transform2::from_scale(glyph_size / svg_size);

            canvas->get_scene()->append_scene(*(svg_scene->get_scene()), glyph_global_transform * emoji_scale);
        }

        if (text_style.debug) {
            canvas->set_transform(glyph_global_transform);
            canvas->set_line_width(1);

            // Add box.
            // --------------------------------
            Path2d layout_path;
            layout_path.add_rect(g.box);

            canvas->set_stroke_paint(Paint::from_color(ColorU::green()));
            canvas->stroke_path(layout_path);
            // --------------------------------

            // Add bbox.
            // --------------------------------
            Path2d bbox_path;
            bbox_path.add_rect(g.bbox);

            canvas->set_stroke_paint(Paint::from_color(ColorU::red()));
            canvas->stroke_path(bbox_path);
            // --------------------------------
        }
    }

    canvas->restore_state();
}

shared_ptr<Pathfinder::SvgScene> VectorServer::load_svg(const std::string &path) {
    auto bytes = Pathfinder::load_file_as_string(path);

    auto svg_scene = std::make_shared<Pathfinder::SvgScene>();
    svg_scene->load_from_string(bytes, *canvas);

    return svg_scene;
}

} // namespace Flint

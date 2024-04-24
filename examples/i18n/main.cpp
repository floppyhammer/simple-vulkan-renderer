#include <iostream>
#include <random>
#include <stdexcept>

#include "app.h"

using namespace Flint;

using Pathfinder::Vec2;
using Pathfinder::Vec3;

const uint32_t WINDOW_WIDTH = 1280;
const uint32_t WINDOW_HEIGHT = 720;

int main() {
    App app({WINDOW_WIDTH, WINDOW_HEIGHT});

    // Build scene tree. Use a block, so we don't increase ref counts for the node.
    {
        auto root = app.get_tree()->get_root();

        auto margin_container = std::make_shared<MarginContainer>();
        margin_container->set_position({0, 0});
        margin_container->set_size({WINDOW_WIDTH, WINDOW_HEIGHT});
        margin_container->set_margin_all(32);
        margin_container->set_anchor_flag(AnchorFlag::FullRect);
        root->add_child(margin_container);

        auto box_container = std::make_shared<HBoxContainer>();
        box_container->set_separation(16);
        margin_container->add_child(box_container);

        std::string text = "";
        text += "👍😁😂\n";                   // Emoji
        text += "你好世界！\n";                    // Chinese
        text += "こんにちは世界！\n";                 // Japanese
        text += "مرحبا بالعالم!\n";           // Arabic
        text += "ওহে বিশ্ব!\n";               // Bengali
        text += "สวัสดีชาวโลก!\n";            // Thai
        text += "سلام دنیا!\n";               // Persian
        text += "नमस्ते दुनिया!\n";           // Hindi
        text += "Chào thế giới!\n";           // Vietnamese
        text += "שלום עולם!\n\n\n";           // Hebrew
        text += "Hello123!مرحبا٠١٢!你好123！\n"; // Mixed languages

        // No word wrapping.
        {
            auto label = std::make_shared<Label>();
            label->set_text_style(TextStyle{
                ColorU::white(),
                ColorU::red(),
                0,
                false,
                false,
                false,
            });
            label->set_text(text);
            label->container_sizing.expand_h = true;
            label->container_sizing.flag_h = ContainerSizingFlag::Fill;
            box_container->add_child(label);
        }

        // Word wrapping.
        {
            auto label = std::make_shared<Label>();
            label->set_word_wrap(true);
            label->set_custom_minimum_size(Vec2F(200, 600));
            label->set_text_style(TextStyle{
                ColorU::white(),
                ColorU::red(),
                0,
                false,
                false,
                false,
            });
            label->set_text(text);
            label->container_sizing.expand_h = true;
            label->container_sizing.flag_h = ContainerSizingFlag::Fill;

            box_container->add_child(label);
        }
    }

    app.main_loop();

    return EXIT_SUCCESS;
}

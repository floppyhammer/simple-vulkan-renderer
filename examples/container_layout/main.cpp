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

        auto hbox_container = std::make_shared<HStackContainer>();
        hbox_container->set_separation(8);
        hbox_container->set_position({100, 100});
        root->add_child(hbox_container);

        for (int _ = 0; _ < 4; _++) {
            auto button = std::make_shared<Button>();
            hbox_container->add_child(button);

            if (_ == 0) {
                button->container_sizing.expand_h = true;
                button->container_sizing.flag_h = ContainerSizingFlag::ShrinkCenter;

                button->container_sizing.flag_v = ContainerSizingFlag::Fill;
            }
            if (_ == 1) {
                button->container_sizing.flag_v = ContainerSizingFlag::ShrinkStart;
            }
            if (_ == 2) {
                button->container_sizing.flag_v = ContainerSizingFlag::ShrinkCenter;
            }
            if (_ == 3) {
                button->container_sizing.flag_v = ContainerSizingFlag::ShrinkEnd;
            }
        }
        hbox_container->set_size({800, 100});

        auto vbox_container = std::make_shared<VStackContainer>();
        vbox_container->set_separation(8);
        vbox_container->set_position({100, 300});
        root->add_child(vbox_container);

        for (int _ = 0; _ < 4; _++) {
            auto button = std::make_shared<Button>();
            vbox_container->add_child(button);

            if (_ == 0) {
                button->container_sizing.expand_v = true;
                button->container_sizing.flag_v = ContainerSizingFlag::ShrinkCenter;

                button->container_sizing.flag_h = ContainerSizingFlag::Fill;
            }
            if (_ == 1) {
                button->container_sizing.flag_h = ContainerSizingFlag::ShrinkStart;
            }
            if (_ == 2) {
                button->container_sizing.flag_h = ContainerSizingFlag::ShrinkCenter;
            }
            if (_ == 3) {
                button->container_sizing.flag_h = ContainerSizingFlag::ShrinkEnd;
            }
        }
        vbox_container->set_size({200, 300});
        // ----------------------------------------------------
    }

    app.main_loop();

    return EXIT_SUCCESS;
}

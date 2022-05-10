#ifndef FLINT_INPUT_SERVER_H
#define FLINT_INPUT_SERVER_H

#include "../common/math/vec2.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>

namespace Flint {
    enum class InputEventType {
        MouseButton = 0,
        MouseMotion,
        MouseScroll,
        Key,
        Max,
    };

    enum class KeyCode {

    };

    class InputEvent {
    public:
        InputEventType type = InputEventType::Max;

        union Args {
            struct {
                KeyCode key;
                bool pressed;
            } key{};
            struct {
                uint8_t button;
                bool pressed;
                Vec2<float> position;
            } mouse_button;
            struct {
                float delta;
            } mouse_scroll;
            struct {
                Vec2<float> relative;
                Vec2<float> position;
            } mouse_motion;
        } args;

        void consume ();

        bool is_consumed() const;

    private:
        bool consumed = false;
    };

    class InputServer {
    public:
        static InputServer &get_singleton() {
            static InputServer singleton;
            return singleton;
        }

        void attach_callbacks(GLFWwindow *window);

        Vec2<float> cursor_position;

        std::vector<InputEvent> input_queue;

        void clear_queue();
    };
}

#endif //FLINT_INPUT_SERVER_H
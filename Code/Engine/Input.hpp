#pragma once

#include <sol/state.hpp>

#include "Common/Types.hpp"
#include "EngineCommon.hpp"
#include "InputCodes.hpp"

namespace x {
    class Input {
        friend class Mouse;
        X_CLASS_PREVENT_MOVES_COPIES(Input)

    public:
        Input() = default;

        bool GetKeyDown(int key) {
            return _keyStates[key].pressed;
        }

        bool GetKeyUp(int key) {
            return _keyStates[key].released;
        }

        bool GetMouseButtonDown(int button) {
            return _mouseStates[button].pressed;
        }

        bool GetMouseButtonUp(int button) {
            return _mouseStates[button].released;
        }

        X_NODISCARD int GetMouseX() const {
            return _mouseX;
        }

        X_NODISCARD int GetMouseY() const {
            return _mouseY;
        }

        X_NODISCARD f32 GetMouseDeltaX() const {
            return _mouseDeltaX;
        }

        X_NODISCARD f32 GetMouseDeltaY() const {
            return _mouseDeltaY;
        }

        void ResetMouseDeltas() {
            _mouseDeltaX = 0.0f;
            _mouseDeltaY = 0.0f;
        }

    private:
        friend class Game;

        void RegisterLuaGlobals(sol::state& state) {
            state["KEY_A"]         = KeyCode::A;
            state["KEY_B"]         = KeyCode::B;
            state["KEY_C"]         = KeyCode::C;
            state["KEY_D"]         = KeyCode::D;
            state["KEY_E"]         = KeyCode::E;
            state["KEY_F"]         = KeyCode::F;
            state["KEY_G"]         = KeyCode::G;
            state["KEY_H"]         = KeyCode::H;
            state["KEY_I"]         = KeyCode::I;
            state["KEY_J"]         = KeyCode::J;
            state["KEY_K"]         = KeyCode::K;
            state["KEY_L"]         = KeyCode::L;
            state["KEY_M"]         = KeyCode::M;
            state["KEY_N"]         = KeyCode::N;
            state["KEY_O"]         = KeyCode::O;
            state["KEY_P"]         = KeyCode::P;
            state["KEY_Q"]         = KeyCode::Q;
            state["KEY_R"]         = KeyCode::R;
            state["KEY_S"]         = KeyCode::S;
            state["KEY_T"]         = KeyCode::T;
            state["KEY_U"]         = KeyCode::U;
            state["KEY_V"]         = KeyCode::V;
            state["KEY_W"]         = KeyCode::W;
            state["KEY_X"]         = KeyCode::X;
            state["KEY_Y"]         = KeyCode::Y;
            state["KEY_Z"]         = KeyCode::Z;
            state["KEY_1"]         = KeyCode::One;
            state["KEY_2"]         = KeyCode::Two;
            state["KEY_3"]         = KeyCode::Three;
            state["KEY_4"]         = KeyCode::Four;
            state["KEY_5"]         = KeyCode::Five;
            state["KEY_6"]         = KeyCode::Six;
            state["KEY_7"]         = KeyCode::Seven;
            state["KEY_8"]         = KeyCode::Eight;
            state["KEY_9"]         = KeyCode::Nine;
            state["KEY_0"]         = KeyCode::Zero;
            state["KEY_MINUS"]     = KeyCode::Minus;
            state["KEY_EQUAL"]     = KeyCode::Equal;
            state["KEY_BACKSPACE"] = KeyCode::Backspace;
            state["KEY_TAB"]       = KeyCode::Tab;
            state["KEY_HOME"]      = KeyCode::Home;
            state["KEY_LEFT"]      = KeyCode::Left;
            state["KEY_UP"]        = KeyCode::Up;
            state["KEY_RIGHT"]     = KeyCode::Right;
            state["KEY_DOWN"]      = KeyCode::Down;
            state["KEY_ESCAPE"]    = KeyCode::Escape;
            state["KEY_ENTER"]     = KeyCode::Enter;
            state["KEY_SPACE"]     = KeyCode::Space;
            state["KEY_LCTRL"]     = KeyCode::LeftControl;
            state["KEY_RCTRL"]     = KeyCode::RightControl;
            state["KEY_F1"]        = KeyCode::F1;
            state["KEY_F2"]        = KeyCode::F2;
            state["KEY_F3"]        = KeyCode::F3;
            state["KEY_F4"]        = KeyCode::F4;
            state["KEY_F5"]        = KeyCode::F5;
            state["KEY_F6"]        = KeyCode::F6;
            state["KEY_F7"]        = KeyCode::F7;
            state["KEY_F8"]        = KeyCode::F8;
            state["KEY_F9"]        = KeyCode::F9;
            state["KEY_F10"]       = KeyCode::F10;
            state["KEY_F11"]       = KeyCode::F11;
            state["KEY_F12"]       = KeyCode::F12;
            state["MOUSE_LEFT"]    = MouseButton::Left;
            state["MOUSE_RIGHT"]   = MouseButton::Right;
            state["MOUSE_MIDDLE"]  = MouseButton::Middle;

            state.new_usertype<Input>("GameInput",
                                      "GetKeyDown",
                                      &Input::GetKeyDown,
                                      "GetKeyUp",
                                      &Input::GetKeyUp,
                                      "GetMouseButtonDown",
                                      &Input::GetMouseButtonDown,
                                      "GetMouseButtonUp",
                                      &Input::GetMouseButtonUp,
                                      "GetMouseX",
                                      &Input::GetMouseX,
                                      "GetMouseY",
                                      &Input::GetMouseY,
                                      "GetMouseDeltaX",
                                      &Input::GetMouseDeltaX,
                                      "GetMouseDeltaY",
                                      &Input::GetMouseDeltaY);
            state["Input"] = this;
        }

        void UpdateKeyState(int key, bool pressed) {
            if (!_enabled) return;

            _keyStates[key].pressed  = pressed;
            _keyStates[key].released = !pressed;
        }

        void UpdateMouseButtonState(int button, bool pressed) {
            if (!_enabled) return;

            _mouseStates[button].pressed  = pressed;
            _mouseStates[button].released = !pressed;
        }

        void UpdateMousePosition(const int x, const int y) {
            if (!_enabled) return;

            _mouseDeltaX = CAST<f32>(x);
            _mouseDeltaY = CAST<f32>(y);

            constexpr f32 deadZone = 2.5f;  // Might need to tweak this, also frame-rate dependent :(
            if (std::abs(_mouseDeltaX) < deadZone) _mouseDeltaX = 0.0f;
            if (std::abs(_mouseDeltaY) < deadZone) _mouseDeltaY = 0.0f;

            _mouseX += x;
            _mouseY += y;
        }

        void SetEnabled(bool enabled) {
            _enabled = enabled;
        }

        struct KeyState {
            bool pressed  = false;
            bool released = false;
        };

        unordered_map<int, KeyState> _keyStates;
        unordered_map<int, KeyState> _mouseStates;
        int _mouseX = 0, _mouseY = 0;
        f32 _mouseDeltaX = 0.f, _mouseDeltaY = 0.f;
        bool _enabled = true;
    };
}  // namespace x
// Author: Jake Rieger
// Created: 4/20/2025.
//

#pragma once

#include <functional>
#include <imgui.h>
#include "Common/Typedefs.hpp"
#include "Common/Macros.hpp"

namespace x {
    static constexpr u64 kModifier_Ctrl  = X_BIT(0);
    static constexpr u64 kModifier_Shift = X_BIT(1);
    static constexpr u64 kModifier_Alt   = X_BIT(2);
    static constexpr u64 kModifier_Super = X_BIT(3);

    class ShortcutManager {
    public:
        template<typename Func>
        void RegisterShortcut(ImGuiKey key, u64 modifierFlags, Func callback) {
            Shortcut sc {};
            sc.mKey       = key;
            sc.mModifiers = modifierFlags;
            sc.mCallback  = [callback]() { callback(); };
            mShortcuts.push_back(sc);
        }

        void ProcessShortcuts() const {
            for (const auto& sc : mShortcuts) {
                if (IsShortcutPressed(sc.mKey, sc.mModifiers)) {
                    X_LOG_DEBUG("Shortcut pressed");
                    sc.mCallback();
                }
            }
        }

    private:
        struct Shortcut {
            ImGuiKey mKey;
            u64 mModifiers;
            std::function<void()> mCallback;
        };

        vector<Shortcut> mShortcuts;

        static bool IsShortcutPressed(ImGuiKey key, u64 modifierFlags) {
            const ImGuiIO& io = ImGui::GetIO();

            const bool ctrlRequired  = X_CHECKBIT(modifierFlags, 0);
            const bool shiftRequired = X_CHECKBIT(modifierFlags, 1);
            const bool altRequired   = X_CHECKBIT(modifierFlags, 2);
            const bool superRequired = X_CHECKBIT(modifierFlags, 3);

            if (io.KeyCtrl != ctrlRequired || io.KeyShift != shiftRequired || io.KeyAlt != altRequired ||
                io.KeySuper != superRequired) {
                return false;
            }

            return ImGui::IsKeyPressed(key, false);
        }
    };
}  // namespace x

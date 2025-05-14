#pragma once

#include "Common/Typedefs.hpp"
#include <functional>
#include <imgui.h>

namespace x {
    static constexpr size_t kMaxInputLength = 256;

    class DevConsole {
    public:
        DevConsole();
        using CommandHandler = std::function<void(const vector<str>&)>;

    private:
        bool mVisible     = false;
        bool mShouldFocus = false;
        char mInputBuffer[kMaxInputLength];
        vector<str> mItems;
        vector<str> mCommands;
        int mHistoryPos = -1;  // -1: newline, 0..history.size()-1 is browsing history
        vector<str> mHistory;
        unordered_map<str, CommandHandler> mHandlers;

    public:
        void Execute(const char* cmdLine);
        DevConsole& RegisterCommand(const str& name, const CommandHandler& handler);
        void AddLog(const char* fmt, ...);
        void Draw();
        int TextEditCallback(ImGuiInputTextCallbackData* data);
        void ToggleVisible();
        bool IsVisible() const;
    };
}  // namespace x
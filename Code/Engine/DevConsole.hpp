#pragma once

#include "Common/Types.hpp"
#include <functional>
#include <imgui/imgui.h>

namespace x {
    static constexpr size_t kMaxInputLength = 256;

    class DevConsole {
    public:
        DevConsole();
        using CommandHandler = std::function<void(const vector<str>&)>;

    private:
        bool _visible = true;
        char _inputBuffer[kMaxInputLength];
        vector<str> _items;
        vector<str> _commands;
        int _historyPos = -1; // -1: newline, 0..history.size()-1 is browsing history
        vector<str> _history;
        unordered_map<str, CommandHandler> _handlers;

    public:
        void Execute(const char* cmdLine);
        void RegisterCommand(const str& name, const CommandHandler& handler);
        void AddLog(const char* fmt, ...);
        void Draw();
        int TextEditCallback(ImGuiInputTextCallbackData* data);
        void ToggleVisibility();
        bool IsVisible() const;
    };
}
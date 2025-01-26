#define _CRT_SECURE_NO_WARNINGS

#include "DevConsole.hpp"

#include <cstdarg>
#include <sstream>

#include "Common/Str.hpp"

namespace x {
    DevConsole::DevConsole() {
        _i_ = StrCopy(_inputBuffer, kMaxInputLength, "");

        RegisterCommand("clear",
                        [this](const vector<str>&) {
                            _items.clear();
                        });

        RegisterCommand("help",
                        [this](const vector<str>&) {
                            _items.emplace_back("Commands:");
                            for (const auto& cmd : _commands) {
                                _items.push_back("- " + cmd);
                            }
                        });

        RegisterCommand("echo",
                        [this](const vector<str>& args) {
                            str msg;
                            for (const auto& arg : args) {
                                msg += arg + " ";
                                AddLog("%s", msg.c_str());
                            }
                        });
    }

    void DevConsole::Execute(const char* cmdLine) {
        _items.push_back(str("> ") + cmdLine);

        std::stringstream ss(cmdLine);
        str command;
        ss >> command;

        vector<str> args;
        str arg;
        while (ss >> arg) {
            args.push_back(arg);
        }

        if (_handlers.contains(command)) {
            _handlers[command](args);
        } else {
            _items.push_back("Unknown command: " + command);
        }

        _history.emplace_back(cmdLine);
        _historyPos = -1;

        _i_ = StrCopy(_inputBuffer, kMaxInputLength, "");
    }

    void DevConsole::RegisterCommand(const str& name, const CommandHandler& handler) {
        _handlers[name] = handler;
        _commands.push_back(name);
    }

    void DevConsole::AddLog(const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);

        _i_ = vsnprintf(buffer, IM_ARRAYSIZE(buffer), fmt, args);

        buffer[IM_ARRAYSIZE(buffer) - 1] = 0;
        va_end(args);
        _items.emplace_back(buffer);
    }

    void DevConsole::Draw() {
        if (!IsVisible()) {
            return;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y * 0.3f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.5f});
        if (!ImGui::Begin("##console",
                          &_visible,
                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                          ImGuiWindowFlags_NoTitleBar)) {
            ImGui::End();
            return;
        }
        ImGui::PopStyleColor();

        ImGui::BeginChild("ScrollingRegion",
                          ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                          false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& item : _items) {
            ImGui::TextUnformatted(item.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        // Command line
        bool reclaimFocus                  = false;
        ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue |
                                             ImGuiInputTextFlags_CallbackCompletion |
                                             ImGuiInputTextFlags_CallbackHistory;

        if (_shouldFocus) {
            ImGui::SetKeyboardFocusHere();
            _shouldFocus = false;
        }

        ImGui::Text(">");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.0f, 0.0f, 0.0f, 0.0f});
        if (ImGui::InputText(
            "##Input",
            _inputBuffer,
            IM_ARRAYSIZE(_inputBuffer),
            inputTextFlags,
            [](ImGuiInputTextCallbackData* data) -> int {
                const auto console = CAST<DevConsole*>(data->UserData);
                return console->TextEditCallback(data);
            },
            this)) {
            char* input_end = _inputBuffer + StrLen(_inputBuffer, kMaxInputLength);
            while (input_end > _inputBuffer && input_end[-1] == ' ')
                input_end--;
            *input_end = 0;

            if (_inputBuffer[0])
                Execute(_inputBuffer);
            reclaimFocus = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        ImGui::SetItemDefaultFocus();
        if (reclaimFocus)
            ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();
    }

    int DevConsole::TextEditCallback(ImGuiInputTextCallbackData* data) {
        switch (data->EventFlag) {
            case ImGuiInputTextFlags_CallbackHistory: {
                const int prevHistoryPos = _historyPos;
                if (data->EventKey == ImGuiKey_UpArrow) {
                    if (_historyPos == -1)
                        _historyPos = CAST<i32>(_history.size()) - 1;
                    else if (_historyPos > 0)
                        _historyPos--;
                } else if (data->EventKey == ImGuiKey_DownArrow) {
                    if (_historyPos != -1) {
                        if (++_historyPos >= _history.size())
                            _historyPos = -1;
                    }
                }

                if (prevHistoryPos != _historyPos) {
                    data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen =
                                                                 snprintf(data->Buf,
                                                                          CAST<size_t>(data->BufSize),
                                                                          "%s",
                                                                          (_historyPos >= 0)
                                                                              ? _history[_historyPos].c_str()
                                                                              : "");
                    data->BufDirty = true;
                }
                break;
            }
        }
        return 0;
    }

    void DevConsole::ToggleVisible() {
        _visible     = !_visible;
        _shouldFocus = _visible;
    }

    bool DevConsole::IsVisible() const {
        return _visible;
    }
}
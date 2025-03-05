#include "DevConsole.hpp"
#include "Common/Str.hpp"
#include <cstdarg>
#include <sstream>

namespace x {
    DevConsole::DevConsole() {
        std::ignore = StrCopy(mInputBuffer, kMaxInputLength, "");

        RegisterCommand("clear", [this](const vector<str>&) { mItems.clear(); });

        RegisterCommand("help", [this](const vector<str>&) {
            mItems.emplace_back("Commands:");
            for (const auto& cmd : mCommands) {
                mItems.push_back("- " + cmd);
            }
        });

        RegisterCommand("echo", [this](const vector<str>& args) {
            str msg;
            for (const auto& arg : args) {
                msg += arg + " ";
                AddLog("%s", msg.c_str());
            }
        });
    }

    void DevConsole::Execute(const char* cmdLine) {
        mItems.push_back(str("> ") + cmdLine);

        std::stringstream ss(cmdLine);
        str command;
        ss >> command;

        vector<str> args;
        str arg;
        while (ss >> arg) {
            args.push_back(arg);
        }

        if (mHandlers.contains(command)) {
            mHandlers[command](args);
        } else {
            mItems.push_back("Unknown command: " + command);
        }

        mHistory.emplace_back(cmdLine);
        mHistoryPos = -1;

        std::ignore = StrCopy(mInputBuffer, kMaxInputLength, "");
    }

    DevConsole& DevConsole::RegisterCommand(const str& name, const CommandHandler& handler) {
        mHandlers[name] = handler;
        mCommands.push_back(name);

        return *this;
    }

    void DevConsole::AddLog(const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);

        std::ignore = vsnprintf(buffer, IM_ARRAYSIZE(buffer), fmt, args);

        buffer[IM_ARRAYSIZE(buffer) - 1] = 0;
        va_end(args);

        const auto msg = str(buffer);
        memset(buffer, 0, sizeof(buffer));

        std::ignore = snprintf(buffer, sizeof(buffer), "[LOG] %s", msg.c_str());
        mItems.emplace_back(buffer);
    }

    void DevConsole::Draw() {
        if (!IsVisible()) { return; }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y * 0.3f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.67f});
        if (!ImGui::Begin("##console",
                          &mVisible,
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
        for (const auto& item : mItems) {
            ImGui::TextUnformatted(item.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        // Command line
        bool reclaimFocus                  = false;
        ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue |
                                             ImGuiInputTextFlags_CallbackCompletion |
                                             ImGuiInputTextFlags_CallbackHistory;

        if (mShouldFocus) {
            ImGui::SetKeyboardFocusHere();
            mShouldFocus = false;
        }

        ImGui::Text(">");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.0f, 0.0f, 0.0f, 0.0f});
        if (ImGui::InputText(
              "##Input",
              mInputBuffer,
              IM_ARRAYSIZE(mInputBuffer),
              inputTextFlags,
              [](ImGuiInputTextCallbackData* data) -> int {
                  const auto console = CAST<DevConsole*>(data->UserData);
                  return console->TextEditCallback(data);
              },
              this)) {
            char* input_end = mInputBuffer + StrLen(mInputBuffer, kMaxInputLength);
            while (input_end > mInputBuffer && input_end[-1] == ' ')
                input_end--;
            *input_end = 0;

            if (mInputBuffer[0]) Execute(mInputBuffer);
            reclaimFocus = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        ImGui::SetItemDefaultFocus();
        if (reclaimFocus) ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();
    }

    int DevConsole::TextEditCallback(ImGuiInputTextCallbackData* data) {
        switch (data->EventFlag) {
            case ImGuiInputTextFlags_CallbackHistory: {
                const int prevHistoryPos = mHistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow) {
                    if (mHistoryPos == -1) mHistoryPos = CAST<i32>(mHistory.size()) - 1;
                    else if (mHistoryPos > 0) mHistoryPos--;
                } else if (data->EventKey == ImGuiKey_DownArrow) {
                    if (mHistoryPos != -1) {
                        if (++mHistoryPos >= mHistory.size()) mHistoryPos = -1;
                    }
                }

                if (prevHistoryPos != mHistoryPos) {
                    data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen =
                      snprintf(data->Buf,
                               CAST<size_t>(data->BufSize),
                               "%s",
                               (mHistoryPos >= 0) ? mHistory[mHistoryPos].c_str() : "");
                    data->BufDirty = true;
                }
                break;
            }
        }
        return 0;
    }

    void DevConsole::ToggleVisible() {
        mVisible     = !mVisible;
        mShouldFocus = mVisible;
    }

    bool DevConsole::IsVisible() const {
        return mVisible;
    }
}  // namespace x
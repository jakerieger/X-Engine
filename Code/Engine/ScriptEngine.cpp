#include "ScriptEngine.hpp"
#include <ranges>

namespace x {
    ScriptEngine& ScriptEngine::Get() {
        static ScriptEngine instance;
        return instance;
    }

    sol::state& ScriptEngine::GetLuaState() {
        return _lua;
    }

    bool ScriptEngine::ExecuteFile(const str& filename) {
        try {
            _lua.script_file(filename);
            return true;
        } catch (const sol::error&) {
            return false;
        }
    }

    bool ScriptEngine::ExecuteString(const str& script) {
        try {
            _lua.script(script);
            return true;
        } catch (const sol::error&) {
            return false;
        }
    }

    ScriptEngine::ScriptEngine() {
        InitializeLua();
    }

    void ScriptEngine::InitializeLua() {
        _lua.open_libraries(sol::lib::base,
                            sol::lib::math,
                            sol::lib::string,
                            sol::lib::table,
                            sol::lib::debug);

        for (const auto& registration : _registeredTypes | std::views::values) {
            registration();
        }
    }
}
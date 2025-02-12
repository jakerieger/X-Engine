#pragma once

#include "EngineCommon.hpp"
#include "Common/Types.hpp"

#include <typeindex>
#include <functional>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace x {
    // Default register handler is empty, types implement this themselves
    template<typename T>
    void RegisterTypeImpl(sol::usertype<T>&) {}

    class ScriptEngine {
        X_CLASS_PREVENT_MOVES_COPIES(ScriptEngine)

    public:
        static ScriptEngine& Get();
        sol::state& GetLuaState();

        bool ExecuteFile(const str& filename);
        bool ExecuteString(const str& script);

        template<typename T>
        void RegisterType(const str& name) {
            _registeredTypes[std::type_index(typeid(T))] = [this, name]() {
                auto usertype = _lua.new_usertype<T>(name);
                RegisterTypeImpl(usertype);
            };
        }

    private:
        ScriptEngine();
        void InitializeLua();

        unordered_map<std::type_index, std::function<void()>> _registeredTypes;
        sol::state _lua;
    };
}

#define X_REGISTER_SCRIPT_TYPE(Type, Name) \
    namespace { \
        struct Type##Registrar { \
            Type##Registrar() { \
                x::ScriptEngine::Get().RegisterType<Type>(Name); \
            } \
        }; \
        static Type##Registrar Type##Instance; \
    }
#pragma once

#include "EngineCommon.hpp"
#include "Common/Types.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace x {
    struct BehaviorEntity;

    template<typename T, typename = void>
    struct LuaTypeTraits {
        static constexpr std::string_view typeName = T::kLuaTypeName;

        static void RegisterMembers(sol::usertype<T>& usertype) {
            T::RegisterMembers(usertype);
        }
    };

    template<typename T>
    struct LuaTypeRegistration {};

    struct LuaRegisterable {
        virtual ~LuaRegisterable() = default;
        virtual void RegisterWithLua(sol::state& lua) const = 0;
    };

    template<typename T>
    struct LuaRegistry : LuaRegisterable {
        static constexpr std::string_view typeName = LuaTypeTraits<T>::typeName;

        void RegisterWithLua(sol::state& lua) const override {
            auto usertype = lua.new_usertype<T>(std::string{typeName});
            LuaTypeTraits<T>::RegisterMembers(usertype);
        }
    };

    struct BehaviorScriptContext {
        sol::environment env;
        sol::protected_function onAwake;
        sol::protected_function onUpdate;
        sol::protected_function onDestroyed;
    };

    // TODO: Add more types of scripts that can be implemented to expand the capabilities of the script engine
    enum class ScriptType : u8 {
        Behavior,
        ScriptTypeCount,
    };

    class ScriptEngine {
        X_CLASS_PREVENT_MOVES_COPIES(ScriptEngine)

    public:
        static ScriptEngine& Get() {
            static ScriptEngine instance;
            return instance;
        }

        sol::state& GetLuaState() {
            return _lua;
        }

        bool LoadScript(const str& source,
                        const str& scriptId,
                        const ScriptType type = ScriptType::Behavior) {
            try {
                auto env = sol::environment(_lua, sol::create, _lua.globals());
                _lua.script(source, env);

                if (type == ScriptType::Behavior) {
                    sol::protected_function awakeFunc     = env["onAwake"];
                    sol::protected_function updateFunc    = env["onUpdate"];
                    sol::protected_function destroyedFunc = env["onDestroyed"];

                    _behaviorContexts[scriptId] = {std::move(env),
                                                   std::move(awakeFunc),
                                                   std::move(updateFunc),
                                                   std::move(destroyedFunc)};
                }

                return true;
            } catch (const sol::error&) {
                return false;
            }
        }

        void CallAwakeBehavior(const str& scriptId,
                               const BehaviorEntity& entity) {
            if (!_behaviorContexts.contains(scriptId)) {
                X_PANIC(
                    "Could not find associated script in behavior contexts for id '%s'",
                    scriptId.c_str());
            }

            const auto& context = _behaviorContexts[scriptId];
            if (context.onAwake.valid()) {
                try {
                    _i_ = context.onAwake(entity);
                } catch (const sol::error& e) { X_PANIC(e.what()); }
            }
        }

        void CallUpdateBehavior(const str& scriptId,
                                f32 deltaTime,
                                const BehaviorEntity& entity) {
            if (!_behaviorContexts.contains(scriptId)) {
                X_PANIC(
                    "Could not find associated script in behavior contexts for id '%s'",
                    scriptId.c_str());
            }

            const auto& context = _behaviorContexts[scriptId];
            if (context.onUpdate.valid()) {
                try {
                    _i_ = context.onUpdate(deltaTime, entity);
                } catch (const sol::error& e) {
                    X_PANIC(e.what());
                }
            }
        }

        void CallDestroyedBehavior(const str& scriptId,
                                   const BehaviorEntity& entity) {
            if (!_behaviorContexts.contains(scriptId)) {
                X_PANIC(
                    "Could not find associated script in behavior contexts for id '%s'",
                    scriptId.c_str());
            }

            const auto& context = _behaviorContexts[scriptId];
            if (context.onDestroyed.valid()) {
                try {
                    _i_ = context.onDestroyed(entity);
                } catch (const sol::error& e) { X_PANIC(e.what()); }
            }
        }

        bool ExecuteFile(const str& filename) {
            try {
                _lua.script_file(filename);
                return true;
            } catch (const sol::error&) { return false; }
        }

        bool Execute(const str& script) {
            try {
                _lua.script(script);
                return true;
            } catch (const sol::error&) { return false; }
        }

        template<typename T>
        void RegisterType() {
            LuaRegistry<T>{}.RegisterWithLua(_lua);
        }

        template<typename... Types>
        void RegisterTypes() {
            (RegisterType<Types>(), ...);
        }

    private:
        ScriptEngine() {
            InitializeLua();
        }

        void InitializeLua() {
            _lua.open_libraries(sol::lib::base,
                                sol::lib::math,
                                sol::lib::string,
                                sol::lib::table,
                                sol::lib::debug);
        }

        sol::state _lua;
        unordered_map<str, BehaviorScriptContext> _behaviorContexts;
    };
}
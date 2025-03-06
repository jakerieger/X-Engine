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
        virtual ~LuaRegisterable()                          = default;
        virtual void RegisterWithLua(sol::state& lua) const = 0;
    };

    template<typename T>
    struct LuaRegistry : LuaRegisterable {
        static constexpr std::string_view typeName = LuaTypeTraits<T>::typeName;

        void RegisterWithLua(sol::state& lua) const override {
            auto usertype = lua.new_usertype<T>(std::string {typeName});
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
        ScriptEngine() {
            InitializeLua();
        }

        sol::state& GetLuaState() {
            return mLua;
        }

        bool LoadScript(const str& source, const str& scriptId, const ScriptType type = ScriptType::Behavior) {
            try {
                auto env = sol::environment(mLua, sol::create, mLua.globals());
                mLua.script(source, env);

                if (type == ScriptType::Behavior) {
                    sol::protected_function awakeFunc     = env["onAwake"];
                    sol::protected_function updateFunc    = env["onUpdate"];
                    sol::protected_function destroyedFunc = env["onDestroyed"];

                    mBehaviorContexts[scriptId] = {std::move(env),
                                                   std::move(awakeFunc),
                                                   std::move(updateFunc),
                                                   std::move(destroyedFunc)};
                }

                return true;
            } catch (const sol::error&) {
                X_LOG_ERROR("Failed to load script: %s", scriptId.c_str());
                return false;
            }
        }

        bool LoadScript(const vector<u8>& bytecode, const str& scriptId, const ScriptType type = ScriptType::Behavior) {
            try {
                auto env = sol::environment(mLua, sol::create, mLua.globals());

                struct ReaderState {
                    const vector<u8>* bytecode {nullptr};
                    size_t position {0};
                };

                ReaderState state;
                state.bytecode = &bytecode;

                auto reader = [](lua_State* L, void* data, size_t* size) -> const char* {
                    ReaderState* state = CAST<ReaderState*>(data);
                    if (state->position >= state->bytecode->size()) {
                        *size = 0;
                        return nullptr;
                    }

                    *size              = state->bytecode->size() - state->position;
                    const char* result = RCAST<const char*>(state->bytecode->data() + state->position);
                    state->position    = state->bytecode->size();
                    return result;
                };

                sol::load_result loadedChunk = mLua.load(reader, &state, scriptId.c_str(), sol::load_mode::binary);
                if (!loadedChunk.valid()) {
                    sol::error err = loadedChunk;
                    return false;
                }

                auto result = loadedChunk(env);
                if (!result.valid()) {
                    sol::error err = result;
                    return false;
                }

                if (type == ScriptType::Behavior) {
                    sol::protected_function awakeFunc     = env["onAwake"];
                    sol::protected_function updateFunc    = env["onUpdate"];
                    sol::protected_function destroyedFunc = env["onDestroyed"];

                    mBehaviorContexts[scriptId] = {std::move(env),
                                                   std::move(awakeFunc),
                                                   std::move(updateFunc),
                                                   std::move(destroyedFunc)};
                }

                return true;
            } catch (const sol::error&) {
                X_LOG_ERROR("Failed to load script: %s", scriptId.c_str());
                return false;
            }
        }

        void CallAwakeBehavior(const str& scriptId, const BehaviorEntity& entity) {
            if (!mBehaviorContexts.contains(scriptId)) {
                X_PANIC("Could not find associated script in behavior contexts for id '%s'", scriptId.c_str());
            }

            const auto& context = mBehaviorContexts[scriptId];
            if (context.onAwake.valid()) {
                try {
                    std::ignore = context.onAwake(entity);
                } catch (const sol::error& e) { X_PANIC(e.what()); }
            }
        }

        void CallUpdateBehavior(const str& scriptId, f32 deltaTime, const BehaviorEntity& entity) {
            if (!mBehaviorContexts.contains(scriptId)) {
                X_PANIC("Could not find associated script in behavior contexts for id '%s'", scriptId.c_str());
            }

            const auto& context = mBehaviorContexts[scriptId];
            if (context.onUpdate.valid()) {
                try {
                    std::ignore = context.onUpdate(deltaTime, entity);
                } catch (const sol::error& e) { X_PANIC(e.what()); }
            }
        }

        void CallDestroyedBehavior(const str& scriptId, const BehaviorEntity& entity) {
            if (!mBehaviorContexts.contains(scriptId)) {
                X_PANIC("Could not find associated script in behavior contexts for id '%s'", scriptId.c_str());
            }

            const auto& context = mBehaviorContexts[scriptId];
            if (context.onDestroyed.valid()) {
                try {
                    std::ignore = context.onDestroyed(entity);
                } catch (const sol::error& e) { X_PANIC(e.what()); }
            }
        }

        bool ExecuteFile(const str& filename) {
            try {
                mLua.script_file(filename);
                return true;
            } catch (const sol::error&) { return false; }
        }

        bool Execute(const str& script) {
            try {
                mLua.script(script);
                return true;
            } catch (const sol::error&) { return false; }
        }

        template<typename T>
        void RegisterType() {
            LuaRegistry<T> {}.RegisterWithLua(mLua);
        }

        template<typename... Types>
        void RegisterTypes() {
            (RegisterType<Types>(), ...);
        }

    private:
        void InitializeLua() {
            mLua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::debug);
        }

        sol::state mLua;
        unordered_map<str, BehaviorScriptContext> mBehaviorContexts;
    };
}  // namespace x
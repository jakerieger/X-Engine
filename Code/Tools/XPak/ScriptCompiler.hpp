// Author: Jake Rieger
// Created: 3/5/2025.
//

#pragma once

#include "Common/Typedefs.hpp"

extern "C" {
#include <lua.h>
}

namespace x {
    class ScriptCompiler {
    public:
        static vector<u8> Compile(const str& source, const str& chunkName);

    private:
        typedef struct {
            vector<u8>* bytecode;
        } BytecodeWriterState;

        static int BytecodeWriter(lua_State* L, const void* p, size_t size, void* ud);
    };
}  // namespace x

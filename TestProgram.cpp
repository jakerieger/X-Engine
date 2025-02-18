#include <sol/sol.hpp>
#include <DirectXTex.h>

int main() {
    sol::state lua;
    lua.open_libraries();
    lua.script("print('Hello from Lua!')");
}
#include <sol/sol.hpp>

int main() {
    sol::state lua;
    lua.open_libraries();
    lua.script("print('Hello from Lua!')");
}
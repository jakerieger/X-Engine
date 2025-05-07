![](Docs/branding/XEngineBanner.png)

---

**XEN**GINE (or XEN for short) is a custom 3D game engine built for Windows from the ground up as both a learning project and fully functional toolsuite for developing games.

It boasts a modest forward renderer, entity component system, fully integrated editor, and an asset management system for packing and shipping game content.

Entity behavior and scripting is handled by a JIT-compiled Lua runtime (thanks to LuaJIT), and the engine core itself is quite lightweight and modular, compiling to a small static library that can be linked to any Windows C++ application one wishes.

## Modules

### X

**X** is the **core engine library**. It's compiled as a static library and almost every other module in this
project links to it. 

> Its source can be found in [Code/Engine](Code/Engine).

### XEditor

**XEditor** is the primary editor and game authoring tool. It provides a GUI that is fully integrated with the engine
and is comparable in nature to the Unity editor or Unreal Engine's editor.

> Its source can be found in [Code/Tools/XEditor](Code/Tools/XEditor).

### XPak

**XPak** is an asset packer and management tool. It's split into two components: **xpak**, which is a static
library containing the asset management API used by the engine core and XEditor, and **xpakc**, which is a command-line utility that can be used to
process assets outside XEditor. Its core functions include generating asset descriptor files, packing assets for distribution,
and unpacking or parsing pak files for debugging purposes.

> Its source can be found in [Code/Tools/XPak](Code/Tools/XPak).

### ResPak

**ResPak** is an internal command-line utility used to compress and pack image resources into header files
so they can be easily embedded in other GUI tools such as XEditor. It uses Brotli for compression.

> Its source can be found in [Code/Tools/ResPak](Code/Tools/ResPak).


## Installing

While no installers are provided at the moment, the libraries can be installed and used in external CMake
projects via `FindPackage()`.

Simply run the following command from the current build target's output directory (i.e. `~/XENGINE/build/Debug`):
```bash
$ cmake --install .
```

This will default to installing XENGINE in `C:\Program Files (x86)\XENGINE`. All you need to do now in order to use it
is to add the following to your external project's `CMakeLists.txt`:
```cmake
# Make sure C++20 is enabled!
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If you installed to the default directory, this is all you need.
# If you installed to a custom directory, you'll need to add the `PATHS` arg and provide the location:
# find_package(XENGINE REQUIRED PATHS C:/XENGINE) for example
find_package(XENGINE REQUIRED)

add_executable(your_project ...)

# Link against the core engine and XPak (required for asset processing)
target_link_libraries(your_project PRIVATE X::x X::xpak)
```

Now you can use XENGINE in your own project code:

```c++
#include <XENGINE/Engine/Window.hpp>

class GameWindow final : public x::IWindow {
public:
    GameWindow(const x::str &title, int width, int height, WORD windowIcon)
        : IWindow(title, width, height, windowIcon) {
    }
};

int main() {
    GameWindow game("MyGame", 800, 600 ,0);
    return game.Run();
}
```
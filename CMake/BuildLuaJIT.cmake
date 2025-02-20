cmake_policy(PUSH)
cmake_policy(SET CMP0169 OLD)

function(build_luajit)
    FetchContent_GetProperties(luajit)

    if (NOT luajit_POPULATED)
        FetchContent_Populate(luajit)

        if (WIN32)
            # Create a custom target for building LuaJIT
            add_custom_command(
                OUTPUT
                "${luajit_SOURCE_DIR}/src/lua51.lib"
                "${luajit_SOURCE_DIR}/src/lua51.exp"
                COMMAND cmd /c msvcbuild.bat static
                WORKING_DIRECTORY ${luajit_SOURCE_DIR}/src
                COMMENT "Building LuaJIT"
                # These files are used by msvcbuild.bat, so we should depend on them
                DEPENDS
                "${luajit_SOURCE_DIR}/src/msvcbuild.bat"
                "${luajit_SOURCE_DIR}/src/luajit.h"
                "${luajit_SOURCE_DIR}/src/lua.h"
            )

            # Create a custom target that depends on the output
            add_custom_target(luajit_build
                DEPENDS "${luajit_SOURCE_DIR}/src/lua51.lib"
            )
        else ()
            message(FATAL_ERROR "Cannot compile LuaJIT for system not supported by this project. Please use Windows.")
        endif ()
    endif ()

    add_library(luajit STATIC IMPORTED GLOBAL)
    add_dependencies(luajit luajit_build)

    set_target_properties(luajit PROPERTIES
        IMPORTED_LOCATION "${luajit_SOURCE_DIR}/src/lua51.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${luajit_SOURCE_DIR}/src"
    )
endfunction()

cmake_policy(POP)
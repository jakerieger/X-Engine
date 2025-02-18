function(build_luajit)
    FetchContent_GetProperties(luajit)

    if(NOT luajit_POPULATED)
        FetchContent_Populate(luajit)

        if(WIN32)
            execute_process(
                COMMAND msvcbuild.bat static
                WORKING_DIRECTORY ${luajit_SOURCE_DIR}/src
                RESULT_VARIABLE LUAJIT_BUILD_RESULT
            )

            if(NOT LUAJIT_BUILD_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to build LuaJIT. Build process returned: ${LUAJIT_BUILD_RESULT}")
            endif()
        else()
            message(FATAL_ERROR "Cannot compile LuaJIT for system not supported by this project. Please use Windows.")
        endif()
    endif()

    add_library(luajit STATIC IMPORTED GLOBAL)

    set_target_properties(luajit PROPERTIES
        IMPORTED_LOCATION "${luajit_SOURCE_DIR}/src/lua51.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${luajit_SOURCE_DIR}/src"
    )
endfunction()
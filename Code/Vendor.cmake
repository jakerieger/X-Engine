file(GLOB IMGUI_SOURCES
    ${CODE_DIR}/Vendor/imgui/*.h
    ${CODE_DIR}/Vendor/imgui/*.cpp
    ${CODE_DIR}/Vendor/imgui/backends/imgui_impl_dx11*
    ${CODE_DIR}/Vendor/imgui/backends/imgui_impl_win32*
)

set(IMGUI_INCLUDE
    ${CODE_DIR}/Vendor/imgui
    ${CODE_DIR}/Vendor/imgui/backends
)

set(STB_SOURCES
    ${CODE_DIR}/Vendor/stb_image.h
    ${CODE_DIR}/Vendor/stb_image_write.h
    ${CODE_DIR}/Vendor/stb_image.cpp
)

set(VENDOR_INCLUDE
    ${CODE_DIR}/Vendor
)
set(VENDOR_DIR ${CODE_DIR}/Vendor)

file(GLOB IMGUI_SOURCES
    ${VENDOR_DIR}/imgui/*.h
    ${VENDOR_DIR}/imgui/*.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_dx11*
    ${VENDOR_DIR}/imgui/backends/imgui_impl_win32*
)

set(IMGUI_INCLUDE
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/imgui/backends
)

set(IMGUI_NODE_EDITOR_SOURCES
    ${VENDOR_DIR}/imgui_node_editor.cpp
)

set(STB_SOURCES
    ${VENDOR_DIR}/stb_image.h
    ${VENDOR_DIR}/stb_image_write.h
    ${VENDOR_DIR}/stb_image.cpp
)

set(RAPIDXML_INCLUDE
    ${VENDOR_DIR}/rapidxml.hpp
    ${VENDOR_DIR}/rapidxml_iterators.hpp
    ${VENDOR_DIR}/rapidxml_print.hpp
    ${VENDOR_DIR}/rapidxml_utils.hpp
)

set(VENDOR_INCLUDE
    ${VENDOR_DIR}
)
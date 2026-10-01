cmake_minimum_required(VERSION 3.20)
project(MO4Desktop LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Libraries are bundled: no dependency downloads during configuration.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory(vendor/glfw)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL ${CMAKE_DL_LIBS})

add_executable(mo4
    src/main.cpp
    src/Application.cpp
    src/Theme.cpp
    src/Desktop.cpp
    src/Taskbar.cpp
    src/Apps.cpp
    src/TaskManager.cpp
)
target_include_directories(mo4 PRIVATE src)
target_link_libraries(mo4 PRIVATE imgui)
if(MSVC)
    target_compile_options(mo4 PRIVATE /W4 /permissive-)
else()
    target_compile_options(mo4 PRIVATE -Wall -Wextra -Wpedantic)
endif()
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT mo4)

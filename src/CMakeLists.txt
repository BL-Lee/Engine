cmake_minimum_required(VERSION 3.10)

#imgui stuff
project(ImGui)

include_directories(${CMAKE_SOURCE_DIR}/dependencies/imgui-master/build)
add_library(ImGui STATIC imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp mimgui_widgets.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp)

#set project
project(Engine)

#set include dirs
include_directories(dependenies/GLFW/include)
include_directories(dependenies/GLEW/include)
include_directories(dependenies/OBJ-Loader)
include_directories(dependenies/stb)
include_directories(dependenies/imgui-master)
include_directories(dependenies/msdfgen-atlas-gen)
include_directories(dependenies/perlin-noise/src)

#set sources
file(GLOB SOURCES "src/*.cpp")

set ( PROJECT_LINK_LIBS ${CMAKE_SOURCE_DIR}dependencies/GLFW/lib-x86_64/libglfw3.a ${CMAKE_SOURCE_DIR}dependencies/GLEW/lib/libGLEW.a ${CMAKE_SOURCE_DIR}imgui.a )

#add the executable name
add_executable(Engine ${SOURCES})
#target_link_libraries(Engine ${PROJECT_LINK_LIBS))

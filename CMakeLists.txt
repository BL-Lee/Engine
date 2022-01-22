cmake_minimum_required(VERSION 3.10)

#imgui stuff
project(imgui)

set ( IMGUI_DIR ${CMAKE_SOURCE_DIR}/dependencies/imgui-master )
add_library(imgui STATIC ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp ${IMGUI_DIR}/imgui_impl_glfw.cpp ${IMGUI_DIR}/imgui_impl_opengl3.cpp)

target_include_directories(imgui PUBLIC dependencies/GLFW/include)

#set project
project(Engine)


#set sources
file(GLOB SOURCES "src/*.cpp")

set ( PROJECT_LINK_LIBS ${CMAKE_SOURCE_DIR}/dependencies/GLFW/lib-x86_64/libglfw3.a ${CMAKE_SOURCE_DIR}/dependencies/GLEW/lib/libGLEW.a imgui )

#add the executable name
add_executable(Engine src/Application.cpp)

#set include dirs
target_include_directories(Engine PUBLIC dependencies/GLFW/include)
target_include_directories(Engine PUBLIC dependencies/GLEW/include)
target_include_directories(Engine PUBLIC dependencies/OBJ-Loader)
target_include_directories(Engine PUBLIC dependencies/stb)
target_include_directories(Engine PUBLIC dependencies/imgui-master)
target_include_directories(Engine PUBLIC dependencies/msdfgen-atlas-gen)
target_include_directories(Engine PUBLIC dependencies/perlin-noise/src)

target_link_libraries(Engine "-framework Cocoa")
target_link_libraries(Engine "-framework OpenGL")
target_link_libraries(Engine "-framework IOKit")

target_link_libraries(Engine ${PROJECT_LINK_LIBS})

set (CMAKE_CXX_FLAGS "-g -DDEBUG_ASSERT")
#add_definitions("DEBUG_ASSERT")

set_target_properties( Engine
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

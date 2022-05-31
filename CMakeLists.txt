cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
project(anomaly)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Raw data generator
add_executable(ad_gen)
target_sources(ad_gen PRIVATE
  src/gen.cpp
  src/pack.cpp
  src/math.cpp
)

target_include_directories(ad_gen PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)


# Featurize binary
add_executable(ad_featurize)
target_sources(ad_featurize PRIVATE
  src/feature.cpp
  src/pack.cpp
  src/math.cpp
)

target_include_directories(ad_featurize PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)


# Training binary
add_executable(ad_train)
target_sources(ad_train PRIVATE
  src/train.cpp
  src/math.cpp
  src/som.cpp
  src/ini.cpp
  src/utils.cpp
)

target_include_directories(ad_train PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)

# GUI
add_executable(ad_gui)
target_sources(ad_gui PRIVATE
  src/gui/main.cpp
  src/gui/ogl.cpp
  src/som.cpp
  src/math.cpp
  src/utils.cpp
  src/ini.cpp
  src/gui/glad.c
  src/gui/imgui.cpp
  src/gui/imgui_demo.cpp
  src/gui/imgui_draw.cpp
  src/gui/imgui_tables.cpp
  src/gui/imgui_widgets.cpp
  src/gui/imgui_impl_glfw.cpp
  src/gui/imgui_impl_opengl3.cpp
)

target_include_directories(ad_gui PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
  "${CMAKE_CURRENT_LIST_DIR}/include/imgui"
  "/usr/local/include"
  "/opt/homebrew/include"
)

target_link_directories(ad_gui PRIVATE
  "/usr/local/lib"
  "/opt/homebrew/lib"
  "${CMAKE_CURRENT_LIST_DIR}/lib"
)

find_package(OpenGL REQUIRED)

if (MSVC)
   add_library(glfw SHARED IMPORTED)
   set_property(TARGET glfw PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/glfw3.dll)
   set_property(TARGET glfw PROPERTY IMPORTED_IMPLIB ${CMAKE_CURRENT_LIST_DIR}/lib/glfw3dll.lib)
   add_custom_command(TARGET ad_gui
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/lib/glfw3.dll ${CMAKE_CURRENT_LIST_DIR}/build/Debug
   )
endif()

target_link_libraries(ad_gui PRIVATE
  glfw
  OpenGL::GL)

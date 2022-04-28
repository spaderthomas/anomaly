cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
project(anomaly)


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
)

target_include_directories(ad_train PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)

# GUI
add_executable(ad_gui)
target_sources(ad_gui PRIVATE
  src/gui/main.cpp
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
)

target_link_directories(ad_gui PRIVATE
  "/usr/local/lib"
)

find_library(OGL OpenGL)
if (NOT OGL)
    message(FATAL_ERROR "OpenGL not found")
endif()

target_link_libraries(ad_gui PRIVATE
  glfw
  ${OGL})

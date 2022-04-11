cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
project(anomaly)


# Raw data generator
add_executable(ad_gen)
target_sources(ad_gen PRIVATE
  src/gen.cpp
  src/pack.cpp
)

target_include_directories(ad_gen PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)


# Featurize binary
add_executable(ad_featurize)
target_sources(ad_featurize PRIVATE
  src/feature.cpp
  src/pack.cpp
)

target_include_directories(ad_featurize PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)


# Training binary
add_executable(ad_train)
target_sources(ad_train PRIVATE
  src/train.cpp
)

target_include_directories(ad_train PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/include"
)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(WITH_GSTREAMER ON)

cmake_minimum_required(VERSION 3.0)

project(test)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(jetsonCamera jetsonCamera.cpp)

target_link_libraries(jetsonCamera ${OpenCV_LIBS})

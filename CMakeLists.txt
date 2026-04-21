cmake_minimum_required(VERSION 4.1)
project(pathtracer_template)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

find_package(OpenMP REQUIRED)

add_executable(pathtracer_template
        main.cpp
        stb_image.h
        stb_image_write.h)

target_link_libraries(pathtracer_template OpenMP::OpenMP_CXX)

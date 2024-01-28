cmake_minimum_required(VERSION 3.16.3)
project(draw_cube)

set (CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})

set(draw_cube_src
    src/main.cpp
   )
add_executable(draw_cube ${draw_cube_src})
target_link_libraries(draw_cube
    ${OpenCV_LIBRARIES}
    )

target_compile_options(draw_cube
    PRIVATE -O3 -std=c++11
    )



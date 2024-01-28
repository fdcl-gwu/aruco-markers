cmake_minimum_required(VERSION 3.16.3)
project(camera_calibration)

set (CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})

set(camera_calibration_src
    src/main.cpp
   )
add_executable(camera_calibration ${camera_calibration_src})
target_link_libraries(camera_calibration
    ${OpenCV_LIBRARIES}
    )

target_compile_options(camera_calibration
    PRIVATE -O3 -std=c++11
    )



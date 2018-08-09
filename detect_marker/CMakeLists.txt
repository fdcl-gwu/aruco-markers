cmake_minimum_required(VERSION 2.4)
project(detect_markers)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 11)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})

set(detect_markers_src
    src/main.cpp
   )
add_executable(detect_markers ${detect_markers_src})
target_link_libraries(detect_markers
    ${OpenCV_LIBRARIES}
    )

target_compile_options(detect_markers
    PRIVATE -O3 -std=c++11
    )



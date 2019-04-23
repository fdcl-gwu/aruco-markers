cmake_minimum_required(VERSION 2.6)
project(opengl_cube)

option(BUILD_OGL43 "Bild OpenGL 4.3 examples" OFF)
mark_as_advanced(BUILD_OGL43)

set (CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)

add_subdirectory(../libraries/glfw glfw)
add_subdirectory(../libraries/glxw glxw)

include_directories(../libraries/glfw/include)
include_directories(../libraries/glm)
include_directories(${CMAKE_BINARY_DIR}/glxw/include)
 
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")
SET(opengl_libs glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS})

link_directories (${OPENGLEXAMPLES_BINARY_DIR}/bin)

add_executable (opengl_cube 
    src/main.cpp
    )
target_link_libraries(opengl_cube ${opengl_libs} )
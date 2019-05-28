// include user headers
#include "cube.hpp"

// include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "load_shaders.hpp"
#include "opengl_config.hpp"


int main(void)
{
    cube_data cube;
    int width, height;
    bool success;

    GLFWwindow *window;

    width = 1024;
    height = 768;

    success = config_window(window, height, width);
    if (!success) return 1;

    GLuint program_id, matrix_id;
    glm::mat4 projection, view;
    config_shaders_cameras(program_id, matrix_id, projection, view);

    // background color
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    // model matrix : an identity matrix (model will be at the origin)
    glm::mat4 model = glm::mat4(1.0f);

    // ModelViewProjection : multiplication of our 3 matrices
    // Remember, matrix multiplication is the other way around
    glm::mat4 MVP = projection * view * model;

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(cube.vertex), cube.vertex, GL_STATIC_DRAW
    );

    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(cube.color), cube.color, GL_STATIC_DRAW
    );
 
    float deg2rad = 3.14159265359f / 180.f;

    do
    {
        // get the time in seconds
        float t = glfwGetTime();

        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use our shader
        glUseProgram(program_id);
        
        // set transformations
        float rotate_speed = 100.0f;
        glm::vec3 euler_angles(
            rotate_speed * t * deg2rad, // rotation around x-axis
            rotate_speed * t * deg2rad, // rotation around y-axis
            rotate_speed * t * deg2rad  // rotation around z- axis
        );

        glm::vec3 translation_vec(
            -10.0f, 
            0.0f, 
            0.0f
        );

        float scale = 1.0f;
        glm::vec3 scale_vec(scale, scale, scale);

        model = glm::mat4(1.0f);
        transform_model(model, translation_vec, euler_angles, scale_vec);

        glm::mat4 MVP = projection * view * model;

        // send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,        // attribute, no particular reason for 0, 
                      //    but must match the layout in the shader
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );

        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
            1,        // attribute, no particular reason for 1, 
                      //     but must match the layout in the shader
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );

        // draw the triangle
        // 12*3 indices starting at 0 -> 12 triangles
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0);

    // cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(program_id);
    glDeleteVertexArrays(1, &vertex_array_id);

    // close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

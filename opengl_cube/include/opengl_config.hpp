#ifndef OPENGL_CONFIG_HPP
#define OPENGL_CONFIG_HPP

#include "load_shaders.hpp"

// include GLEW
#include <GL/glew.h>

// include GLFW
#include <GLFW/glfw3.h>

// include GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


bool config_window(GLFWwindow *&window, const int height, const int width);

void config_shaders_cameras(
    GLuint &program_id, GLuint &matrix_id, glm::mat4 &projection,
    glm::mat4 &view
);

void transform_model(
    glm::mat4 &model, const glm::vec3 &translation_vec,
    const glm::vec3 &euler_angles, const glm::vec3 &scale_vec
);

void set_buffer(GLuint &buffer, const GLfloat points[]);

#endif
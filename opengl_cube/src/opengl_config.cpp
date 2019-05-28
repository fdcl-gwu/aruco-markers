#include "opengl_config.hpp"


bool config_window(GLFWwindow *&window, const int height, const int width)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(width, height, "OpenGL Cube", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU,"
            "they are not 3.3 compatible. Try the 2.1 version of the "
            "tutorials.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    return true;
}

void config_shaders_cameras(
    GLuint &program_id, GLuint &matrix_id, glm::mat4 &projection, 
    glm::mat4 & view
)
{
    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Create and compile our GLSL program from the shaders
    program_id = LoadShaders(
        "../TransformVertexShader.vertexshader",
        "../ColorFragmentShader.fragmentshader"
    );

    // Get a handle for our "MVP" uniform
    matrix_id = glGetUniformLocation(program_id, "MVP");

    // projection matrix 
    projection = glm::perspective(
        glm::radians(45.0f), // 45 degree Field of view,
        4.0f / 3.0f,         // 4:3 ratio,
        0.1f,                // display range : 0.1 unit <-> 100 units
        100.0f
    );

    // Camera matrix
    view = glm::lookAt(
        glm::vec3(0.0, 0.0, 0.0), // Camera is at (0, 0, 0), in World Space
        glm::vec3(-1.0, 0.0, 0.0),  // and looks at -x axis
        glm::vec3(0.0, 1.0, 0.0)   // Head is up, (0,-1,0) to look upside-down)
    );
}

void transform_model(
    glm::mat4 &model, const glm::vec3 &translation_vec,
    const glm::vec3 &euler_angles, const glm::vec3 &scale_vec
)
{
    glm::quat rot_quat = glm::quat(euler_angles);
    glm::mat4 rotation_matrix = glm::toMat4(rot_quat);

    glm::mat4 translation_matrix = glm::translate(translation_vec);

    glm::mat4 scale_matrix = glm::scale(scale_vec);

    model = translation_matrix * rotation_matrix * scale_matrix * model;
}

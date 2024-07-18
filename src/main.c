#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/types-struct.h>

#include "render_base/shader.h"

#include <stdio.h>
#include <stdbool.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window, bool *wireframe);
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow *window, double x_off, double y_off);

const vec3 up_unit = {0.0f, 1.0f, 0.0f};
vec3 camera_pos = {0.0f, 0.0f, -0.5f};
vec3 camera_target = {0.0f, 0.0f, 0.0f};
vec3 camera_front = {0.0f, 0.0f, -1.0f};
const float camera_speed = 2.5f;

float delta_time = 0.0f;
float last_frame = 0.0f;

float yaw;
float pitch;
float last_x;
float last_y;
float fov = 90.0f;

vec3 light_pos = {1.2f, 1.0f, 2.0f};

int main(void) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "learn opengl", NULL, NULL);
    if(window == NULL) {
        fprintf(stderr, "failed to create glfw window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "failed to initialize glad\n");
        glfwTerminate();
        return 1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    unsigned int shader_program = shader_make("src/render_base/shader.vs", "src/render-base/shader.fs");
    vec3 object_colour = {1.0f, 0.5f, 0.31f};

    glEnable(GL_DEPTH_TEST);
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    unsigned int light_VAO;
    glGenVertexArrays(1, &light_VAO);
    glBindVertexArray(light_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    bool wireframe = false;

    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_lookat(
        (vec3) {0.0f, 0.0f, -0.5f},
        (vec3) {0.0f, 0.0f, 0.0f},
        (vec3) {0.0f, 1.0f, 0.0f},
        view
    );

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    last_x = width/2;
    last_y = height/2;

    while(!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        process_input(window, &wireframe);

        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // matrix stuff
        vec3 temp;
        glm_vec3_copy(camera_front, temp);
        glm_vec3_add(camera_pos, camera_front, temp);

        glm_lookat(
            camera_pos,
            temp,
            up_unit,
            view
        );

        mat4 projection;
        glm_perspective(glm_rad(fov), (float)width/(float)height, 0.1f, 100.0f, projection);

        mat4 model = GLM_MAT4_IDENTITY_INIT;

        // rendering
        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glUniform3fv(glGetUniformLocation(shader_program, "objectColour"), 1, object_colour);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        glm_translate(model, light_pos);
        glm_scale_uni(model, 0.2f);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &model[0][0]);
        glBindVertexArray(light_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &light_VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window, bool *wireframe) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        *wireframe = !*wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, *wireframe ? GL_LINE : GL_FILL);
    }

    // movement
    glm_vec3_scale(camera_front, camera_speed * delta_time, camera_front);
    vec3 temp_camera_front;
    glm_vec3_copy(camera_front, temp_camera_front);

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        glm_vec3_add(camera_pos, camera_front, camera_pos);
    if(glfwGetKey(window, GLFW_KEY_DOWN))
        glm_vec3_sub(camera_pos, camera_front, camera_pos);
    if(glfwGetKey(window, GLFW_KEY_LEFT)) {
        glm_vec3_cross(camera_front, up_unit, temp_camera_front);
        glm_vec3_sub(camera_pos, temp_camera_front, camera_pos);
    } if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
        glm_vec3_cross(camera_front, up_unit, temp_camera_front);
        glm_vec3_add(camera_pos, temp_camera_front, camera_pos);
    }

    glm_vec3_normalize(camera_front);
}

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
    float x_off = x_pos - last_x;
    float y_off = last_y - y_pos;
    last_x = x_pos;
    last_y = y_pos;

    const float sensitivity = 0.1f;
    x_off *= sensitivity;
    y_off *= sensitivity;

    yaw += x_off;
    pitch += y_off;

    if(pitch > 89.0f)
        pitch = 89.0f;
    else if(pitch < -89.0f)
        pitch = -89.0f;

    vec3 dir;
    dir[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));   // x
    dir[1] = sin(glm_rad(pitch));                       // y
    dir[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));   // z
    glm_vec3_normalize(dir);
    glm_vec3_copy(dir, camera_front);
}

void scroll_callback(GLFWwindow *window, double x_off, double y_off) {
    fov -= (float)y_off;
    if(fov < 1.0f)
        fov = 1.0f;
    else if(fov > 180.0f)
        fov = 180.0f;
}

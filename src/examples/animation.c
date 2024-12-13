#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include <engine/general.h>
#include <engine/shader.h>
#include <engine/camera.h>
#include <engine/renderer.h>
#define ANIMATION_UTILITIES
#include <engine/animation.h>

void animation_action(
        struct Animation *_animation,
        struct Actor *actor,
        float t,
        void *_extra_args
) {
    printf("t: %f\n", t);
   ((Rectangle*)actor->actor)->x = lerp_f32(0.f, 10.f, t);
};

int main(void) {
    GLFWwindow *window = init_window("Familiar animation example");
    if(window == NULL) return 1;

    Animation animation;
    animation_init(&animation);
    animation.duration = 10;
    animation.renderer->camera.position.z = -10;
    set_camera_callback(window, &animation.renderer->camera);

    Rectangle object = (Rectangle){0, 0, 5, 5};
    Actor *actor = animation_actor_add(&animation, (void *)&object, OBJECT_RECTANGLE, 0);
    Action action = (Action){
        .start = 2,
        .end = 6,
        .mutate_actor = animation_action
    };
    animation_action_add(actor, &action);

    bool started = false;

    // wait until start
    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_switch_3d(animation.renderer);
        render_draw_rectangle(animation.renderer, (Rectangle){0, 0, 10, 10}, 0);
        render_frame_flush(animation.renderer);

        if(glfwGetKey(window, GLFW_KEY_S)) started = true;

        update_delta_time();
        if(started)
            animation_play(&animation);

        process_general_input(window);
        process_camera_input(window, &animation.renderer->camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    animation_free(&animation);
    glfwTerminate();
    return 0;
}

#include "animation.h"

// Animator

void animator_init(Animator *animator) {
    *animator = (Animator){0};

    arrsetcap(animator->objects, 10);
}

void animator_free(Animator *animator) {
    arrfree(animator->objects);
}

void animator_play(Animator *animator) {
    if(animator->time == animator->duration)
        return;
    animator->time++;

    for(size_t i = 0; i < arrlen(animator->objects); i++)
        animator_object_play(&animator->objects[i], animator->time);
}

void animator_object_play(Object *object, float time) {
    size_t *actions = animator_current_actions(object, time);

    for(size_t i = 0; i < actions[0]; i++)
        object->actions[i].mutate_object(object);

    free(actions);

    // render object (object->actions[0])
}

void animator_object_add(
    Animator *animator,
    void *actor,
    const size_t actor_size,
    float time
) {
    Object object = (Object){
        .object = actor,
        .object_size = actor_size
    };
    arrsetcap(object.actions, 3);

    // actions[0] is always the lifetime of the object
    const Action object_action = (Action){
        .start = time,
        .end = time,
        .mutate_object = NULL
    };
    arrput(object.actions, object_action);

    const size_t index = animator_object_place(animator, time);
    arrins(animator->objects, index, object);
}

void animator_object_action_add(Object *object, Action action) {
    const size_t index = animator_action_place(object, action.start);
    arrins(object->actions, index, action);
}

size_t animator_object_place(Animator *animator, float time) {
    size_t l = 0;
    size_t r = arrlen(animator->objects) - 1;
    while(l < r) {
        const size_t m = (r - l)/2;
        const float object = animator->objects[m].actions[0].start;

        if(time > object)
            l = m;
        else if(time < object)
            r = m;
        else if(time == object) {
            l = m;
            break;
        }
    }

    return l;
}

size_t animator_action_place(Object *object, float time) {
    size_t l = 0;
    size_t r = arrlen(object->actions) - 1;
    while(l < r) {
        const size_t m = (r - l)/2;
        const float action = object->actions[m].start;

        if(time > action)
            l = m;
        else if(time < action)
            r = m;
        else if(time == action) {
            l = m;
            break;
        }
    };

    return l;
}

// needs to be freed
// first size_t is the amount of elements
size_t *animator_current_actions(Object *object, float time) {
    const size_t start = animator_action_place(object, time);
    size_t end = start;

    size_t buffer[arrlen(object->actions)];
    for(;
        end < arrlen(object->actions) &&
        time >= object->actions[end].start &&
        time <= object->actions[end].end;
        end++
    )
        buffer[end-start] = end;

    size_t *elements = malloc((end-start) * sizeof(size_t));
    elements[0] = end-start;
    memcpy(&elements[1], buffer, end-start);
    return elements;
}

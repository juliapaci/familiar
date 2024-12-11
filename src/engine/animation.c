#include "animation.h"

void animation_init(Animation *animation, Renderer *renderer) {
    *animation = (Animation){.renderer = renderer, 0};

    arrsetcap(animation->objects, 10);
}

// NOTE: does not free animation.renderer
void animation_free(Animation *animation) {
    arrfree(animation->objects);
}

void animation_render_object(Animation *animation, Object *object) {
    switch(object->kind) {
        case OBJECT_TRIANGLE: {

        } break;

        case OBJECT_CIRCLE: {
            render_draw_circle(animation->renderer, *(Circle *)object->object, 0);
        } break;

        case OBJECT_RECTANGLE: {
            render_draw_rectangle(animation->renderer, *(Rectangle *)object->object, 0);
        } break;

        case OBJECT_CUBE: {
            render_draw_cube(animation->renderer, *(Cube *)object->object, 0);
        } break;

        default: break;
    }
}

void animation_play(Animation *animation) {
    if(animation->time == animation->duration)
        return;
    animation->time++;

    for(size_t i = 0; i < arrlen(animation->objects); i++)
        animation_object_play(animation, &animation->objects[i], animation->time);
}

void animation_object_play(Animation *animation, Object *object, float time) {
    size_t *actions = animation_active_actions(object, time);

    for(size_t i = 1; i < actions[0]; i++) // object->actions[0] will always be lifetime so we can just skip it
        object->actions[i].mutate_object(
            object,
            (time - object->actions[i].start)/(object->actions[i].end - object->actions[i].start)
        );

    free(actions);

    animation_render_object(animation, object);
}

void animation_object_add(
    Animation *animation,
    void *actor,
    const ObjectKind kind,
    float time
) {
    Object object = (Object){
        .object = actor,
        .kind = kind
    };
    arrsetcap(object.actions, 3);

    // actions[0] is always the lifetime of the object
    const Action object_action = (Action){
        .start = time,
        .end = time,
        .mutate_object = NULL
    };
    arrput(object.actions, object_action);

    const size_t index = animation_object_place(animation, time);
    arrins(animation->objects, index, object);
}

void animation_object_action_add(Object *object, Action action) {
    const size_t index = animation_action_place(object, action.start);
    arrins(object->actions, index, action);
}

size_t animation_object_place(Animation *animation, float time) {
    size_t l = 0;
    size_t r = arrlen(animation->objects) - 1;
    while(l < r) {
        const size_t m = (r - l)/2;
        const float object = animation->objects[m].actions[0].start;

        if(time > object)
            l = m;
        else if(time < object)
            r = m;
        else if(time == object) {
            return m;
        }
    }

    return l;
}

size_t animation_action_place(Object *object, float time) {
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
            return m;
        }
    };

    return l;
}

// needs to be freed
// first size_t is the amount of elements
size_t *animation_active_actions(Object *object, float time) {
    const size_t start = animation_action_place(object, time);
    size_t end;

    size_t buffer[arrlen(object->actions)];
    buffer[0] = start;
    for(end = start + 1;
        end < arrlen(object->actions) &&
        time >= object->actions[end].start &&
        time <= object->actions[end].end;
        end++
    )
        buffer[end-start] = end;

    size_t *elements = malloc((end-start) * sizeof(size_t));
    elements[0] = end-start;
    memcpy(elements + 1, buffer, end-start);
    return elements;
}

#ifdef ANIMATION_UTILITIES

float lerp_f32(float a, float b, float t) {
    return a * (1.f - t) + (b * t);
}

#endif // ANIMATION_UTILITIES

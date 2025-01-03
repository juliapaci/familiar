#include "animation.h"

void animation_init(Animation *animation) {
    *animation = (Animation){0};

    animation->renderer = malloc(sizeof(Renderer));
    render_init(animation->renderer);
    arrsetcap(animation->actors, 10);
}

void animation_free(Animation *animation) {
    render_free(animation->renderer);
    free(animation->renderer);
    arrfree(animation->actors);
}

void animation_render_actor(Animation *animation, Actor *actor) {
    // TODO: better switching between circle and triangle renderer states
    switch(actor->kind) {
        case OBJECT_TRIANGLE: {

        } break;

        case OBJECT_CIRCLE: {
            render_draw_circle(animation->renderer, *(Circle *)actor->actor);
        } break;

        case OBJECT_RECTANGLE: {
            render_draw_rectangle(animation->renderer, *(Rectangle *)actor->actor, 1);
        } break;

        case OBJECT_CUBE: {
            render_draw_cube(animation->renderer, *(Cube *)actor->actor, 1);
        } break;

        default: break;
    }
}

void animation_play(Animation *animation) {
    if(animation->time != animation->duration)
        animation->time++;

    for(size_t i = 0; i < arrlen(animation->actors); i++)
        animation_actor_play(animation, &animation->actors[i], animation->time);
}

extern void _animation_action_default(struct Animation *animation, struct Actor *actor, float _t, void *_extra_args);

void animation_actor_play(Animation *animation, Actor *actor, float time) {
    size_t *actions = animation_active_actions(actor, time);

    for(size_t i = 0; i < actions[0]; i++)
        actor->actions[i].mutate_actor(
            animation,
            actor,
            (time - actor->actions[i].start)/(actor->actions[i].end - actor->actions[i].start),
            NULL
        );

    free(actions);
}

Actor *animation_actor_add(
    Animation *animation,
    void *object,
    const ObjectKind kind,
    float time
) {
    Actor actor = (Actor){
        .actor = object,
        .kind = kind
    };
    arrsetcap(actor.actions, 3);

    // actions[0] is always the lifetime of the actor
    const Action default_action = (Action){
        .start = time,
        .end = time,
        .mutate_actor = _animation_action_default
    };
    arrput(actor.actions, default_action);

    const size_t index = animation_actor_place(animation, time);
    arrins(animation->actors, index, actor);

    return animation->actors + index;
}

void animation_action_add(Actor *actor, Action *action) {
    // TODO: auto adjust Animation duration if action exceeds the end
    const size_t index = animation_action_place(actor, action->start);
    arrins(actor->actions, index, (*action));
}

// TODO: macro is easier but a generic function would be more safe
#define _ANIMATION_BINARY_SEARCH(array, retrieval, target) do {     \
    printf("%ld\n", arrlen(array));\
    if(arrlen(array) == 0)                                          \
        return 0;                                                   \
                                                                    \
    size_t l = 0;                                                   \
    size_t r = arrlen(array) - 1;                                   \
    while(l < r) {                                                  \
        const size_t m = (r + l)/2;                                 \
        const float element = array[m].retrieval;                   \
                                                                    \
        if(time > element)                                          \
            l = m;                                                  \
        else if(time < element)                                     \
            r = m;                                                  \
        else if(time == element)                                    \
            return m + 1;                                           \
    }                                                               \
                                                                    \
    return l + 1;                                                   \
} while(0)

size_t animation_actor_place(Animation *animation, float time) {
    _ANIMATION_BINARY_SEARCH(animation->actors, actions[0].start, time);
}

size_t animation_action_place(Actor *actor, float time) {
    _ANIMATION_BINARY_SEARCH(actor->actions, start, time);
}

// needs to be freed
// first size_t is the amount of elements
size_t *animation_active_actions(Actor *actor, float time) {
    const size_t start = animation_action_place(actor, time);
    size_t end;

    size_t buffer[arrlen(actor->actions)];
    buffer[0] = start;
    for(end = start + 1;
        end < arrlen(actor->actions) &&
        time >= actor->actions[end].start &&
        time <= actor->actions[end].end;
        end++
    )
        buffer[end-start] = end;
    for(int i = 0; i < end-start; i++)
        printf("b: %ld\n", buffer[i]);

    size_t *elements = malloc((end-start) * sizeof(size_t));
    elements[0] = end-start;
    memcpy(elements + 1, buffer, end-start);
    return elements;
}

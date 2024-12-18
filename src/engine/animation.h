#ifndef __FAMILIAR_ANIMATION_H__
#define __FAMILIAR_ANIMATION_H__

#define ANIMATION_UTILITIES // TODO: need to define it for now cause of linking errors
// define `ANIMATION_UTILITIES` for utility functions like `lerp`

// NOTE: will probably reimplement in another language for better generic support since void *, macros, and metaprogramming is annoying to use

#include <engine/renderer.h>

#include <stb/stb_ds.h>

// forward declarations
struct Actor;
struct Animation;

typedef struct {
    float start;
    float end;

    // function to act upon the actor
    // `t` should be normalised [0, 1] as progress
    void (*mutate_actor)(
        struct Animation *animation,
        struct Actor *actor,
        float t,
        void *extra_args
    );
} Action;

// TODO: maybe an ecs for transform and other components
// an animated entity
typedef struct Actor {
    // actions will always be sorted by their start time
    // actions[0] will always be lifetime of the actor
    Action *actions; // stb dynamic array

    // object
    // can be any renderer shape like rectangle or something
    void *actor;
    /* const */ ObjectKind kind; // type of object
} Actor;

// animation context
typedef struct Animation {
    Renderer *renderer; // rendering context to draw stuff with

    float time;         // current time (frame)
    float duration;     // duration of animation

    Actor *actors;      // stb dynamic array
} Animation;

// animation
void animation_init(Animation *animation);
void animation_free(Animation *animation);

void animation_render_actor(Animation *animation, Actor *actor);
void animation_play(Animation *animation); // plays an animation frame
void animation_actor_play(Animation *animation, Actor *actor, float time); // plays animation frame
inline void _animation_action_default(
        struct Animation *animation,
        struct Actor *actor,
        float _t,
        void *_extra_args
) { animation_render_actor(animation, actor); };

Actor *animation_actor_add(Animation *animation, void *actor, const ObjectKind kind, float time);
void animation_action_add(Actor *actor, Action *action);

// placements for sorted actors
size_t animation_actor_place(Animation *animation, float time); // index of `animation`'s actors which `time` should sort into
size_t animation_action_place(Actor *actor, float time); // index of `actor`'s actions which `time` should sort into
size_t *animation_active_actions(Actor *actor, float time); // index of the first action which `time` lies within

// animation utilities
#ifdef ANIMATION_UTILITIES
float lerp_f32(float a, float b, float t);
#endif // ANIMATION_UTILITIES

#endif // __FAMILIAR_ANIMATION_H__

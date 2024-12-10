#ifndef __ANIMATION_H__
#define __ANIMATION_H__
// define `ANIMATION_UTILITIES` for utility functions like `lerp`

// NOTE: will probably reimplement in another language for better generic support since void *, macros, and metaprogramming is annoying to use

#include <engine/renderer.h>

#include <stb/stb_ds.h>

struct Object;

// all timing is in 60 frames (1 (float) = 60 (frames))

typedef struct {
    float start;
    float end;

    // function to act upon the object
    // `t` should be normalised [0, 1]
    void (*mutate_object)(struct Object *object, float t);
} Action;

typedef enum {
    OBJECT_RECTANGLE,
    OBJECT_CUBE,
    OBJECT_CIRCLE
} ObjectKind;

// TODO: maybe an ecs for transform and other components
// an animated entity
typedef struct Object {
    // actions will always be sorted by their start time
    // actions[0] will always be lifetime of the object
    Action *actions; // stb dynamic array

    // actor
    // can be any renderer shape like rectangle or something
    void *object;
    /* const */ ObjectKind kind; // type of object
} Object;

// animation context
typedef struct {
    Renderer *renderer; // rendering context to draw stuff with

    float time;         // current time
    float duration;     // duration of animation

    Object *objects;    // stb dynamic array
} Animation;

// animation
void animation_init(Animation *animation, Renderer *renderer);
void animation_free(Animation *animation);

void animation_render_object(Animation *animation, Object *object);
void animation_play(Animation *animation); // plays an animation frame
void animation_object_play(Animation *animation, Object *object, float time); // plays animation frame

void animation_object_add(Animation *animation, void *object, const ObjectKind kind, float time);
void animation_object_action_add(Object *object, Action action);

// binary searches for placements for sorted actors
size_t animation_object_place(Animation *animation, float time); // index of `animation`'s objects which `time` should sort into
size_t animation_action_place(Object *object, float time); // index of `object`'s actions which `time` should sort into
size_t *animation_active_actions(Object *object, float time); // index of the first action which `time` lies within

// animation utilities
#ifdef ANIMATION_UTILITIES
float lerp_f32(float a, float b, float t);
#endif // ANIMATION_UTILITIES

#endif // __ANIMATION_H__

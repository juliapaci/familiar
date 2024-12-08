#ifndef __ANIMATION_H__
#define __ANIMATION_H__

// NOTE: will probably reimplement in another language for better generic support since void *, macros, and metaprogramming is annoying to use

#include <stb/stb_ds.h>

struct Object;

// time is in ms
// TODO: maybe should be in frames instead of time?

typedef struct {
    float start;
    float end;

    // function to act upon the object
    void (*mutate_object)(struct Object *object);
} Action;

// an animated entity
typedef struct Object {
    // actions will always be sorted by their start time
    // actions[0] will always be lifetime of the object
    Action *actions; // stb dynamic array

    // actor
    // can be any renderer shape like rectangle or something
    void *object;
    /* const */ size_t object_size;
} Object;

// animation context
typedef struct {
    float time;
    float duration; // duration of animation

    Object *objects; // stb dynamic array
} Animator;

// Animator
void animator_init(Animator *animator);
void animator_free(Animator *animator);

void animator_play(Animator *animator); // plays an animation frame
void animator_object_play(Object *object, float time); // plays animation frame

void animator_object_add(Animator *animator, void *object, const size_t object_size, float time);
void animator_object_action_add(Object *object, Action action);

// binary searches for placements for sorted actors
size_t animator_object_place(Animator *animator, float time); // index of `animator`'s objects which `time` should sort into
size_t animator_action_place(Object *object, float time); // index of `object`'s actions which `time` should sort into
size_t *animator_current_actions(Object *object, float time); // index of the first action which `time` lies within

#endif // __ANIMATION_H__

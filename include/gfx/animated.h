/*
 * a simple animation timer system
 */
#pragma once

typedef int (*AnimationUpdate)(float progress, void *data);
typedef int (*AnimationCleanup)(int completed, void *data);

typedef struct AnimationConfig
{
    AnimationUpdate on_update;
    AnimationCleanup on_cleanup;
    float duration;
    void *data;
} AnimationConfig;

typedef struct Animation
{
    AnimationConfig config;
    float elapsed;
} Animation;

typedef struct AnimationPointer
{
    int index;
    int generation;
} AnimationPointer;

int anim_create(AnimationConfig config, AnimationPointer *ptr);
int anim_cancel(AnimationPointer animation);

void anim_clear_all();

void anim_update();
void anim_release();

// helper functions
// cubic-bezier(.55,-0.15,.55,1.15) for cards
float anim_cubic_bezier(float progress, float x1, float y1, float x2, float y2);
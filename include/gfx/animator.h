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

float anim_get_duration(AnimationPointer animation);
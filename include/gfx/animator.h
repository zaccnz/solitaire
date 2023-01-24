/*
 * a simple animation timer system
 */
#pragma once

typedef int (*AnimationUpdate)(float progress, void *data);
typedef int (*AnimationRender)(float progress, void *data);
typedef int (*AnimationResize)(int width, int height, void *data);
typedef int (*AnimationCleanup)(int completed, void *data);

typedef struct AnimationConfig
{
    AnimationUpdate on_update;
    AnimationRender on_render;
    AnimationResize on_resize;
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
void anim_render();
void anim_resize();

void anim_release();

void *anim_get_data(AnimationPointer animation);
float anim_get_duration(AnimationPointer animation);
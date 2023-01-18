#include "gfx/animator.h"

#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ANIMATION_MAX 256

Animation *animations[ANIMATION_MAX] = {NULL};
int generations[ANIMATION_MAX] = {0};

int anim_find(AnimationPointer animation)
{
    if (generations[animation.index] != animation.generation)
    {
        printf("cannot cancel animation %d: generation %d does not match %d\n",
               animation.index, animation.generation, generations[animation.index]);
        return -1;
    }
    if (!animations[animation.index])
    {
        printf("cannot cancel animation %d: null\n", animation.index);
        return -1;
    }

    return animation.index;
}

int anim_find_next_free()
{
    int i = 0;

    while (animations[i])
    {
        i++;
        if (i == ANIMATION_MAX)
        {
            printf("out of animations\n");
            return -1;
        }
    }

    return i;
}

void anim_clear(int index, int completed)
{
    Animation *animation = animations[index];

    if (animation->config.on_cleanup)
    {
        animation->config.on_cleanup(completed, animation->config.data);
    }

    free(animations[index]);

    animations[index] = NULL;
    generations[index]++;
}

int anim_create(AnimationConfig config, AnimationPointer *ptr)
{
    Animation *animation = malloc(sizeof(Animation));
    memset(animation, 0, sizeof(Animation));

    animation->config = config;

    int index = anim_find_next_free();
    if (index == -1)
    {
        printf("failed to add animation\n");
        return 0;
    }

    animations[index] = animation;

    if (ptr)
    {
        ptr->index = index;
        ptr->generation = generations[index];
    }

    return 1;
}

int anim_cancel(AnimationPointer animation)
{
    int index = anim_find(animation);
    if (index < 0)
    {
        return 0;
    }

    anim_clear(index, 0);
    return 1;
}

void anim_clear_all()
{
    for (int i = 0; i < ANIMATION_MAX; i++)
    {
        if (!animations[i])
        {
            continue;
        }
        anim_clear(i, 0);
    }
}

void anim_update()
{
    Animation *animation = NULL;
    float delta = GetFrameTime();
    for (int i = 0; i < ANIMATION_MAX; i++)
    {
        if (!animations[i])
        {
            continue;
        }

        animation = animations[i];
        animation->elapsed += delta;
        int looping = (animation->config.duration - 0.0f < 0.0001);
        int result;
        if (animation->config.on_update)
        {
            result = animation->config.on_update(looping ? delta : animation->elapsed / animation->config.duration, animation->config.data);
        }

        if (looping && !result)
        {
            anim_clear(i, 1);
        }
        else if (!looping && animation->elapsed > animation->config.duration)
        {
            anim_clear(i, 1);
        }
    }
}

void anim_release()
{
    for (int i = 0; i < ANIMATION_MAX; i++)
    {
        if (!animations[i])
        {
            continue;
        }

        anim_clear(i, 0);
    }
}

void *anim_get_data(AnimationPointer animation)
{
    int index = anim_find(animation);
    if (index < 0)
    {
        return NULL;
    }

    return animations[index]->config.data;
}

float anim_get_duration(AnimationPointer animation)
{
    int index = anim_find(animation);
    if (index < 0)
    {
        return 0.0f;
    }

    return animations[index]->elapsed / animations[index]->config.duration;
}
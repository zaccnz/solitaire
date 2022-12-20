#include "gfx/animated.h"

#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define ANIMATION_MAX 256

Animation *animations[ANIMATION_MAX] = {NULL};
int generations[ANIMATION_MAX] = {0};

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

int anim_clear(int index, int completed)
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
    int i = animation.index;
    if (generations[i] != animation.generation)
    {
        printf("cannot cancel animation %d: generation %d does not match %d\n",
               i, animation.generation, generations[i]);
        return 0;
    }
    if (!animations[i])
    {
        printf("cannot cancel animation %d: null\n", i);
        return 0;
    }

    anim_clear(i, 0);
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
        if (animation->config.on_update)
        {
            animation->config.on_update(animation->elapsed / animation->config.duration, animation->config.data);
        }

        if (animation->elapsed > animation->config.duration)
        {
            if (animation->config.on_complete)
            {
                animation->config.on_complete(animation->config.data);
            }
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
#include "scenes/scene.h"

#include <stdio.h>
#include <raylib.h>

#define MAX_SCENES 20

const Scene *scenes[MAX_SCENES];
int scene_count = 0;

void scene_push(const Scene *scene)
{
    if (scene_count >= MAX_SCENES)
    {
        printf("scene manager: failed to push new scene, scene list full\n");
        return;
    }

    if (scene_count > 0)
    {
        Scene_PauseFunc pause = scenes[scene_count - 1]->pause;
        if (pause)
            pause();
    }
    scenes[scene_count++] = scene;
    Scene_StartFunc start = scene->start;
    if (start)
        start();
}

void scene_pop()
{
    if (scene_count == 0)
    {
        printf("scene manager: failed to pop scene, scene list empty\n");
    }

    Scene_StopFunc stop = scenes[--scene_count]->stop;
    if (stop)
        stop();

    if (scene_count > 0)
    {
        Scene_PlayFunc play = scenes[scene_count - 1]->play;
        if (play)
            play();
    }
}

void scene_pop_to(const Scene *scene)
{
    if (!scene_is_on_stack(scene))
    {
        printf("scene manager: failed to pop to scene, not on stack\n");
        return;
    }

    while (scenes[scene_count - 1] != scene)
    {
        scene_pop();
    }
}

void scene_pop_all()
{
    while (scene_count > 0)
    {
        scene_pop();
    }
}

void scene_render(struct nk_context *ctx)
{
    if (scene_count == 0)
    {
        DrawText("No scene", 10, 10, 20, LIGHTGRAY);
        return;
    }

    int i = scene_count - 1;
    for (; i >= 0; i--)
    {
        if (!scenes[i]->popup)
        {
            break;
        }
    }
    for (; i < scene_count; i++)
    {
        Scene_RenderFunc render = scenes[i]->render;
        if (render)
            render(ctx);
    }
}

void scene_update(float delta)
{
    if (scene_count == 0)
    {
        return;
    }

    int i = scene_count - 1;
    for (; i >= 0; i--)
    {
        if (!scenes[i]->popup)
        {
            break;
        }
    }
    for (; i < scene_count; i++)
    {
        Scene_UpdateFunc update = scenes[i]->update;
        if (update)
            update(delta, i != (scene_count - 1));
    }
}

int scene_is_on_stack(const Scene *scene)
{
    for (int i = 0; i < scene_count; i++)
    {
        if (scenes[i] == scene)
        {
            return 1;
        }
    }
    return 0;
}
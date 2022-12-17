#include "scene.h"

#include <raylib.h>

#define MAX_SCENES 20

Scene *scenes[MAX_SCENES];
int scene_count = 0;

void scene_push(Scene *scene)
{
    if (scene_count >= MAX_SCENES)
    {
        printf("[scene manager] failed to push new scene, scene list full\n");
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
        printf("[scene manager] failed to pop scene, scene list empty\n");
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

void scene_render()
{
    if (scene_count == 0)
    {
        DrawText("No scene", 10, 10, 20, LIGHTGRAY);
    }
    else
    {
        Scene_RenderFunc render = scenes[scene_count - 1]->render;
        if (render)
            render();
    }
}

void scene_update(double delta)
{
    if (scene_count == 0)
    {
    }
    else
    {
        Scene_UpdateFunc update = scenes[scene_count - 1]->update;
        if (update)
            render(delta);
    }
}
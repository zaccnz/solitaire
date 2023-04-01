#include "scenes/scene.h"

#include <stdio.h>
#include <raylib.h>

#define MAX_SCENES 20

const Scene *scenes[MAX_SCENES];
int scene_count = 0;

typedef enum SCENECHANGE
{
    SCENE_CHANGE_NONE = 0,
    SCENE_CHANGE_PUSH,
    SCENE_CHANGE_POP,
} SceneChange;
struct SceneChangeData
{
    SceneChange type;
    const Scene *scene;
} scene_changes[MAX_SCENES] = {0};
int scene_change_count = 0;

void scene_push(const Scene *scene)
{
    scene_changes[scene_change_count++] = (struct SceneChangeData){
        .type = SCENE_CHANGE_PUSH,
        .scene = scene,
    };
    /*
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
    */
}

void scene_pop()
{
    scene_changes[scene_change_count++] = (struct SceneChangeData){
        .type = SCENE_CHANGE_POP,
        .scene = NULL,
    };

    /*
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
    */
}

void scene_pop_to(const Scene *scene)
{
    int index = scene_stack_pos(scene);
    if (index == -1)
    {
        printf("scene manager: failed to pop to scene, not on stack\n");
        return;
    }

    for (int i = index + 1; i < scene_count; i++)
    {
        scene_pop();
    }
}

void scene_pop_all()
{
    for (int i = 0; i < scene_count; i++)
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
    for (int i = 0; i < scene_change_count; i++)
    {
        switch (scene_changes[i].type)
        {
        case SCENE_CHANGE_POP:
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
            break;
        }
        case SCENE_CHANGE_PUSH:
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
            scenes[scene_count++] = scene_changes[i].scene;
            Scene_StartFunc start = scene_changes[i].scene->start;
            if (start)
                start();
            break;
        }
        default:
        {
            break;
        }
        }
    }
    scene_change_count = 0;

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

int scene_stack_pos(const Scene *scene)
{
    for (int i = 0; i < scene_count; i++)
    {
        if (scenes[i] == scene)
        {
            return i;
        }
    }
    return -1;
}
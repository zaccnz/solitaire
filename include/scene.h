#pragma once

typedef void (*Scene_StartFunc)();
typedef void (*Scene_StopFunc)();
typedef void (*Scene_PlayFunc)();
typedef void (*Scene_PauseFunc)();
typedef void (*Scene_UpdateFunc)(double dt);
typedef void (*Scene_RenderFunc)();

typedef struct Scene
{
    Scene_StartFunc start;
    Scene_StopFunc stop;
    Scene_PlayFunc play;
    Scene_PauseFunc pause;
    Scene_UpdateFunc update;
    Scene_RenderFunc render;
} Scene;

void scene_push(Scene *scene);
void scene_pop();
void scene_render();
void scene_update(double delta);
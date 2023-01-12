#pragma once

typedef void (*Scene_StartFunc)();
typedef void (*Scene_StopFunc)();
typedef void (*Scene_PlayFunc)();
typedef void (*Scene_PauseFunc)();
typedef void (*Scene_UpdateFunc)(float dt, int background);
typedef void (*Scene_RenderFunc)(struct nk_context *ctx);

typedef struct Scene
{
    Scene_StartFunc start;
    Scene_StopFunc stop;
    Scene_PlayFunc play;
    Scene_PauseFunc pause;
    Scene_UpdateFunc update;
    Scene_RenderFunc render;
    int popup;
} Scene;

void scene_push(const Scene *scene);
void scene_pop();
void scene_render(struct nk_context *ctx);
void scene_update(float delta);

extern const Scene GameScene;
extern const Scene MenuScene;
extern const Scene SettingsScene;
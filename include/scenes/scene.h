#pragma once

#include <raylib.h>
#include <raylib-nuklear.h>

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
void scene_pop_to(const Scene *scene);
void scene_pop_all();
void scene_render(struct nk_context *ctx);
void scene_update(float delta);

int scene_is_on_stack(const Scene *scene);

extern const Scene GameScene;
extern const Scene LeaderboardScene;
extern const Scene MenuScene;
extern const Scene SettingsScene;

void game_new_deal(int seed);
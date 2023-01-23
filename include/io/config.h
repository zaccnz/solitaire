#pragma once

typedef struct Config
{
    int animations;
    int fullscreen;
    int sfx;
    struct
    {
        int width, height;
    } window_size;
    struct
    {
        int dealthree;
        int timed;
    } solitaire;
    struct
    {
        struct TextureConfig
        {
            char *pack;
            char *texture_name;
        } background, backs, cards;
    } textures;
    struct
    {
        int render_hitboxes;
        int render_animation_list;
        int render_leaderboard_tool;
        int seed;
    } debug;
} Config;

extern Config config;

void config_load();
void config_free();

void config_push_pack(struct TextureConfig *texture, char *pack, char *texture_name);
void config_save();
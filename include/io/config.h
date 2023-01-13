#pragma once

#define CFG_DEFAULT_PACK "internal"
#define CFG_DEFAULT_TEXTURE_NAME "Default"
#define CFG_DEFAULT_TEXTURE_NAME_BACKS "Red"

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
        struct
        {
            char *pack;
            char *texture_name;
        } background, backs, cards;
    } textures;
    struct
    {
        int render_hitboxes;
        int render_animation_list;
        int seed;
    } debug;
} Config;

extern Config config;

void config_load();
void config_free();

void config_save();
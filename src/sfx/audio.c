#include "sfx/audio.h"

#include "io/config.h"

#include <stdio.h>
#include <raylib.h>

#define SFX_QUEUE_MAX 512

#define SFX_PATH "res/sfx/"
#define SFX_PATH_KENNEY SFX_PATH "kenney/"

typedef struct SFX_Config
{
    SFX sound;
    const char *path;
    float volume;
} SFX_Config;

typedef struct SFX_Queued
{
    SFX sound;
    float delay;
} SFX_Queued;

const SFX_Config SFX_CONFIGS[] = {
    {
        .sound = SFX_DRAW_CARD,
        .path = SFX_PATH_KENNEY "cardSlide2.ogg",
        .volume = 0.7,
    },
    {
        .sound = SFX_DEAL_CARD,
        .path = SFX_PATH_KENNEY "cardPlace1.ogg",
    },
    {
        .sound = SFX_GAME_WIN,
        .path = SFX_PATH "gameWin.mp3",
    },
};

const int SFX_COUNT = sizeof(SFX_CONFIGS) / sizeof(*SFX_CONFIGS);

Sound sfx[SFX_MAX];
SFX_Queued queue[SFX_QUEUE_MAX];
int queue_len = 0;
int sfx_loaded[SFX_MAX] = {0};

void audio_init()
{
    InitAudioDevice();
    for (int i = 0; i < SFX_COUNT; i++)
    {
        SFX_Config config = SFX_CONFIGS[i];
        Sound sound = LoadSound(config.path);
        if (config.volume)
        {
            SetSoundVolume(sound, config.volume);
        }

        sfx[config.sound] = sound;
        sfx_loaded[config.sound] = 1;
    }
    for (int i = 0; i < SFX_MAX; i++)
    {
        if (!sfx_loaded[i])
        {
            printf("audio: missing config for SFX %d\n", i);
        }
    }
}

void audio_free()
{
    for (int i = 0; i < SFX_MAX; i++)
    {
        if (!sfx_loaded[i])
        {
            continue;
        }
        UnloadSound(sfx[i]);
    }
    CloseAudioDevice();
}

void audio_update()
{
    for (int i = queue_len - 1; i >= 0; i--)
    {
        SFX_Queued *queued = &queue[i];
        queued->delay -= GetFrameTime();
        if (queued->delay > 0.0)
        {
            continue;
        }
        audio_play_sfx(queued->sound);
        queue[i] = queue[queue_len - 1];
        queue_len--;
    }
}

void audio_play_sfx(SFX sound)
{
    if (!config.sfx)
    {
        return;
    }

    if (sound >= SFX_MAX)
    {
        printf("audio: invalid sfx %d\n", sound);
        return;
    }
    if (!sfx_loaded[sound])
    {
        printf("audio: playing unloaded sound %d\n", sound);
        return;
    }
    PlaySound(sfx[sound]);
}

void audio_play_sfx_delay(SFX sound, float seconds)
{
    SFX_Queued *queued = &queue[queue_len];
    queued->sound = sound;
    queued->delay = seconds;
    queue_len++;
}
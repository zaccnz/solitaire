#pragma once

typedef enum SFX
{
    SFX_DRAW_CARD = 0,
    SFX_DEAL_CARD,
    SFX_GAME_WIN,
    SFX_MAX,
} SFX;

void audio_init();
void audio_free();

void audio_update();

void audio_play_sfx(SFX sound);
void audio_play_sfx_delay(SFX sound, float seconds);
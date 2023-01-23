#include "gfx/background.h"

#include <raylib.h>

void background_render(Assets *bg_assets)
{
    Texture tex = bg_assets->background.texture;
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    Rectangle source = {
        .x = 0,
        .y = 0,
        .width = tex.width,
        .height = tex.height,
    };

    switch (bg_assets->background.type)
    {
    case BACKGROUND_COVER:
    {
        float image_aspect = (float)tex.width / (float)tex.height;
        float screen_aspect = (float)sw / (float)sh;

        int target_width, target_height;

        if (screen_aspect < image_aspect)
        {
            target_width = (sh / image_aspect);
            target_height = sh;
        }
        else
        {
            target_width = sw;
            target_height = (sw / image_aspect);
        }

        Rectangle dest = {
            .x = (sw - target_width) / 2,
            .y = (sh - target_height) / 2,
            .width = target_width,
            .height = target_height,
        };

        DrawTexturePro(tex, source, dest, (Vector2){.x = 0, .y = 0}, 0.0f, WHITE);
        break;
    }
    case BACKGROUND_STRETCH:
    {
        Rectangle dest2 = {
            .x = 0,
            .y = 0,
            .width = sw,
            .height = sh,
        };

        printf("rendering stretched\n");

        DrawTexturePro(tex, source, dest2, (Vector2){.x = 0, .y = 0}, 0.0f, WHITE);
        break;
    }
    case BACKGROUND_TILE:
    {
        Rectangle dest = {
            .x = 0,
            .y = 0,
            .width = sw,
            .height = sh,
        };

        DrawTextureTiled(tex, source, dest, (Vector2){.x = 0, .y = 0}, 0.0f, 1.0f, WHITE);
        break;
    }
    default:
    {
        printf("invalid background type %d for rendering\n", bg_assets->background.type);
        break;
    }
    }
}
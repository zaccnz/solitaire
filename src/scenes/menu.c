#include "scenes/scene.h"

#include <raylib.h>

#include <stdio.h>

void menu_start()
{
    printf("started menu scene\n");
}

void menu_stop()
{
    printf("stopped menu scene\n");
}

void menu_render(struct nk_context *ctx)
{
    DrawText("main menu", 190, 200, 20, LIGHTGRAY);
}

const Scene MenuScene = {
    .start = menu_start,
    .stop = menu_stop,
    .render = menu_render,
};

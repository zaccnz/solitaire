#include "scenes/menu.h"

#include <raylib.h>

#include <stdio.h>

void start()
{
    printf("started menu scene\n");
}

void stop()
{
    printf("stopped menu scene\n");
}

bool render_button(const char *message, int x, int y, int w, int h)
{
    Vector2 mousePos = GetMousePosition();
    bool hl = (mousePos.x >= x && mousePos.x <= (x + w) &&
               mousePos.y >= y && mousePos.y <= (y + h));

    DrawRectangle(x, y, w, h, hl ? ORANGE : RED);
    DrawText(message, x, y, 16, WHITE);

    return hl && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

void render()
{
    DrawText("main menu", 190, 200, 20, LIGHTGRAY);
    if (render_button("play game", 190, 240, 200, 30))
    {
        printf("play\n");
    }
    if (render_button("quit", 190, 280, 200, 30))
    {
        printf("quit\n");
    }
}

const Scene MenuScene = {
    .start = start,
    .stop = stop,
    .render = render,
};

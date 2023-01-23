#pragma once

#include "solitaire.h"

#include <raylib.h>
#include <raylib-nuklear.h>

void debug_render(struct nk_context *ctx, Solitaire *solitaire);

void debug_hitboxes(struct nk_context *ctx);
void debug_animation_list(struct nk_context *ctx);
void debug_leaderboard_tool(struct nk_context *ctx, Solitaire *solitaire);
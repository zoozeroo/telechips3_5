#pragma once
#include <stdbool.h>
#include "score.h"

typedef struct { float x, y, w, h; } Rect;

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank,
    float mx, float my);
void draw_play(int W, int H, int score, int selected_item);
void draw_howto(int W, int H);
void draw_rank(int W, int H);
void draw_end(int W, int H, const char* name_buf, int score, bool success);

#pragma once
#include <stdbool.h>
#include "score.h"

typedef struct { float x, y, w, h; } Rect;

#define GRID_COLS 9
#define GRID_ROWS 5

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank,
    float mx, float my);
void draw_play(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, const int marks[GRID_ROWS][GRID_COLS]);
void draw_howto(int W, int H);
void draw_rank(int W, int H);
void draw_end(int W, int H, const char* name_buf, int score, bool success);

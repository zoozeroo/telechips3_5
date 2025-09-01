#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define GRID_COLS 7
#define GRID_ROWS 5
#define NAME_MAX 32
#define SCORE_FILE "scores.txt"

typedef struct { float x, y, w, h; } Rect;

int app_run(void);
#pragma once
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

typedef enum {
    STATE_MENU = 0,
    STATE_PLAY,
    STATE_HOWTO,
    STATE_RANK,
    STATE_END
} GameState;

typedef enum {
    RESULT_NONE = 0,
    RESULT_SUCCESS,
    RESULT_FAIL
} GameResult;

typedef struct {
    float x, y, w, h;
} Rect;

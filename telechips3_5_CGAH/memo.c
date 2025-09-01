#if 0
// ================ app.h ================
#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define GRID_COLS 7
#define GRID_ROWS 5
#define NAME_MAX 32
#define SCORE_FILE "scores.txt"

typedef struct { float x, y, w, h; } Rect;

int app_run(void);

// ================ assets.h ================
#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

extern ALLEGRO_BITMAP* bg_home;
extern ALLEGRO_BITMAP* bg_rank;
extern ALLEGRO_BITMAP* bg_play;
extern ALLEGRO_FONT* font_title;
extern ALLEGRO_FONT* font_ui;
extern ALLEGRO_BITMAP* spr_items;
extern ALLEGRO_BITMAP* icon_coffee_1;
extern ALLEGRO_BITMAP* icon_coffee_2;
extern ALLEGRO_BITMAP* icon_coffee_3;
extern ALLEGRO_BITMAP* icon_sleeping;
extern ALLEGRO_BITMAP* icon_people1;
extern ALLEGRO_BITMAP* icon_people2;
extern ALLEGRO_BITMAP* icon_people3;
extern ALLEGRO_BITMAP* icon_coffee_bean;

bool assets_load(void);
void assets_unload(void);

// ================ score.h ================
#pragma once
#define MAX_SCORE 10
#define NAME_MAX 32

typedef struct { char name[NAME_MAX]; int score; } Entry;

void score_load(const char* path);
void score_add_and_save(int s, const char* name, const char* path);
int score_count_get(void);
Entry score_get(int index);

// ================ screens.h ================
#pragma once
#include "app.h"  // Rect 타입을 위해 app.h 포함

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank, float mx, float my);
void draw_play(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, const int marks[GRID_ROWS][GRID_COLS]);
void draw_howto(int W, int H);
void draw_rank(int W, int H);
void draw_end(int W, int H, const char* name_buf, int score_second, bool success);
void draw_play_with_game(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, bool show_ranges);

// ================ game.h ================
#pragma once
#include <stdbool.h>
#include "app.h"  // GRID_ROWS, GRID_COLS를 위해 app.h 포함

#define MAX_STAGES 5
#define KILLS_TO_ADVANCE 10
#define MAX_ENEMIES 50
#define ENEMY_HP 100
#define ENEMY_SPEED 0.8f
#define ATTACK_TOWER_COST 100
#define RESOURCE_TOWER_COST 75
#define TANK_TOWER_COST 50
#define ATTACK_TOWER_RANGE 50.0f
#define ATTACK_TOWER_DAMAGE 25
#define ATTACK_TOWER_COOLDOWN 0.8f
#define RESOURCE_TOWER_AMOUNT 15
#define RESOURCE_TOWER_COOLDOWN 1.0f
#define ATTACK_TOWER_HP 200
#define RESOURCE_TOWER_HP 150
#define TANK_TOWER_HP 400
#define ENEMY_ATTACK_RANGE 18.0f
#define ENEMY_ATTACK_DAMAGE 15
#define ENEMY_ATTACK_COOLDOWN 0.8f

typedef enum { TOWER_EMPTY, TOWER_ATTACK, TOWER_RESOURCE, TOWER_TANK } TowerType;

typedef struct {
    TowerType type;
    float cooldown;
    int hp;
} Tower;

typedef struct {
    bool active;
    float x, y;
    int hp;
    float atk_cooldown;
} Enemy;

typedef struct {
    int caffeine;
    int lives;
    int stage;
    int stage_kills;
    bool cleared;
    bool game_over;
} GameState;

void game_init(void);
void game_update(float dt);
void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges);
void game_place_tower(TowerType type, int row, int col);
void game_sell_tower(int row, int col);
GameState game_get_state(void);
void game_reset(void);

// ================ main.c ================
#include "app.h"

int main(void) {
    return app_run();
}

// ================ app.c ================
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#include "app.h"
#include "assets.h"
#include "score.h"
#include "screens.h"
#include "game.h"

typedef enum { STATE_MENU = 0, STATE_PLAY, STATE_HOWTO, STATE_RANK, STATE_END } AppState;
typedef enum { RESULT_NONE = 0, RESULT_SUCCESS, RESULT_FAIL } GameResult;

static AppState   g_state = STATE_MENU;
static GameResult g_result = RESULT_NONE;

static char   name_buf[NAME_MAX] = { 0 };
static int    name_len = 0;
static bool   end_recorded = false;
static double play_start_time = 0.0;

// 플레이 화면 커서
static int cursor_col = 0, cursor_row = 0;
static int selected_item = 0;
static bool show_all_ranges = false;

static bool point_in_rect(float px, float py, Rect r) {
    return (px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h);
}

int app_run(void) {
    if (!al_init()) return 1;
    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_primitives_addon();
    if (!al_init_image_addon()) return 1;

    const int W = 800, H = 600;
    ALLEGRO_DISPLAY* disp = al_create_display(W, H);
    if (!disp) return 1;

    score_load(SCORE_FILE);
    if (!assets_load()) return 1;

    ALLEGRO_TIMER* frame_timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* q = al_create_event_queue();

    al_register_event_source(q, al_get_timer_event_source(frame_timer));
    al_register_event_source(q, al_get_display_event_source(disp));
    al_register_event_source(q, al_get_keyboard_event_source());
    al_register_event_source(q, al_get_mouse_event_source());
    al_start_timer(frame_timer);

    Rect btn_start = { W / 2.0f - 120, H / 2.0f - 100, 240, 50 };
    Rect btn_howto = { W / 2.0f - 120, H / 2.0f - 25, 240, 50 };
    Rect btn_rank = { W / 2.0f - 120, H / 2.0f + 50, 240, 50 };

    float mx = 0, my = 0;
    int final_score = 0;

    bool running = true, redraw = true;

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(q, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == frame_timer) {
                redraw = true;

                if (g_state == STATE_PLAY) {
                    game_update(1.0f / 60.0f);

                    GameState gs = game_get_state();
                    if (gs.game_over || gs.lives <= 0) {
                        g_result = RESULT_FAIL;
                        final_score = (int)(al_get_time() - play_start_time);
                        g_state = STATE_END;
                    }
                    else if (gs.cleared) {
                        g_result = RESULT_SUCCESS;
                        final_score = (int)(al_get_time() - play_start_time);
                        g_state = STATE_END;
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            mx = ev.mouse.x; my = ev.mouse.y;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (g_state == STATE_MENU) {
                if (point_in_rect(mx, my, btn_start)) {
                    g_state = STATE_PLAY;
                    g_result = RESULT_NONE;
                    final_score = 0;
                    end_recorded = false;
                    name_len = 0; name_buf[0] = '\0';
                    play_start_time = al_get_time();

                    cursor_col = 0; cursor_row = 0;
                    selected_item = 0;
                    show_all_ranges = false;
                    game_reset();
                    game_init();
                }
                else if (point_in_rect(mx, my, btn_howto)) {
                    g_state = STATE_HOWTO; g_result = RESULT_NONE;
                }
                else if (point_in_rect(mx, my, btn_rank)) {
                    g_state = STATE_RANK;  g_result = RESULT_NONE;
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (g_state == STATE_END) {
                int ch = ev.keyboard.unichar;
                if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                    if (name_len > 0) name_buf[--name_len] = '\0';
                }
                else if (ch >= 32 && ch < 127 && ch != ',') {
                    if (name_len < NAME_MAX - 1) {
                        name_buf[name_len++] = (char)ch;
                        name_buf[name_len] = '\0';
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            int key = ev.keyboard.keycode;
            if (key == ALLEGRO_KEY_ESCAPE) running = false;

            if (g_state == STATE_HOWTO) {
                if (key == ALLEGRO_KEY_SPACE) g_state = STATE_MENU;
            }
            else if (g_state == STATE_RANK) {
                if (key == ALLEGRO_KEY_SPACE) g_state = STATE_MENU;
            }
            else if (g_state == STATE_PLAY) {
                // 아이템 선택 (WASD)
                switch (key) {
                case ALLEGRO_KEY_W: selected_item = 0; break;
                case ALLEGRO_KEY_A: selected_item = 1; break;
                case ALLEGRO_KEY_S: selected_item = 2; break;
                case ALLEGRO_KEY_D: selected_item = 3; break;
                }

                // 커서 이동
                if (key == ALLEGRO_KEY_LEFT && cursor_col > 0) cursor_col--;
                if (key == ALLEGRO_KEY_RIGHT && cursor_col < GRID_COLS - 1) cursor_col++;
                if (key == ALLEGRO_KEY_UP && cursor_row > 0) cursor_row--;
                if (key == ALLEGRO_KEY_DOWN && cursor_row < GRID_ROWS - 1) cursor_row++;

                // 타워 설치/판매
                if (key == ALLEGRO_KEY_SPACE) {
                    if (selected_item == 1) game_place_tower(TOWER_ATTACK, cursor_row, cursor_col);
                    else if (selected_item == 2) game_place_tower(TOWER_RESOURCE, cursor_row, cursor_col);
                    else if (selected_item == 3) game_place_tower(TOWER_TANK, cursor_row, cursor_col);
                    else game_sell_tower(cursor_row, cursor_col);
                }

                // 범위 표시 토글
                if (key == ALLEGRO_KEY_R) show_all_ranges = !show_all_ranges;

                // 강제 종료 (테스트용)
                if (key == ALLEGRO_KEY_ENTER) {
                    g_result = RESULT_SUCCESS;
                    final_score = (int)(al_get_time() - play_start_time);
                    g_state = STATE_END;
                }
                else if (key == ALLEGRO_KEY_BACKSPACE) {
                    g_result = RESULT_FAIL;
                    final_score = (int)(al_get_time() - play_start_time);
                    g_state = STATE_END;
                }
            }
            else if (g_state == STATE_END) {
                if (key == ALLEGRO_KEY_ENTER) {
                    if (!end_recorded) {
                        score_add_and_save(final_score, name_buf, SCORE_FILE);
                        end_recorded = true;
                    }
                    g_state = STATE_MENU;
                }
                else if (key == ALLEGRO_KEY_SPACE) {
                    g_state = STATE_MENU;
                }
            }
        }

        if (redraw && al_is_event_queue_empty(q)) {
            switch (g_state) {
            case STATE_MENU:
                draw_menu(W, H, btn_start, btn_howto, btn_rank, mx, my);
                break;
            case STATE_PLAY: {
                int live_sec = (int)(al_get_time() - play_start_time);
                draw_play_with_game(W, H, live_sec, cursor_col, cursor_row, selected_item, show_all_ranges);
                break;
            }
            case STATE_HOWTO: draw_howto(W, H); break;
            case STATE_RANK:  draw_rank(W, H);  break;
            case STATE_END:   draw_end(W, H, name_buf, final_score, (g_result == RESULT_SUCCESS)); break;
            }
            al_flip_display();
            redraw = false;
        }
    }

    assets_unload();
    al_destroy_event_queue(q);
    al_destroy_timer(frame_timer);
    al_destroy_display(disp);
    return 0;
}

// ================ assets.c ================
#include "assets.h"
#include <allegro5/allegro_image.h>

ALLEGRO_BITMAP* bg_home = NULL;
ALLEGRO_BITMAP* bg_rank = NULL;
ALLEGRO_BITMAP* bg_play = NULL;

ALLEGRO_FONT* font_title = NULL;
ALLEGRO_FONT* font_ui = NULL;

ALLEGRO_BITMAP* spr_items = NULL;
ALLEGRO_BITMAP* icon_coffee_1 = NULL;
ALLEGRO_BITMAP* icon_coffee_2 = NULL;
ALLEGRO_BITMAP* icon_coffee_3 = NULL;
ALLEGRO_BITMAP* icon_sleeping = NULL;
ALLEGRO_BITMAP* icon_people1 = NULL;
ALLEGRO_BITMAP* icon_people2 = NULL;
ALLEGRO_BITMAP* icon_people3 = NULL;
ALLEGRO_BITMAP* icon_coffee_bean = NULL;

bool assets_load(void) {
    font_title = al_create_builtin_font();
    font_ui = al_create_builtin_font();
    if (!font_title || !font_ui) return false;

    bg_home = al_load_bitmap("background_play.png");
    bg_rank = al_load_bitmap("background_play.png");
    bg_play = al_load_bitmap("background_play.png");

    spr_items = al_load_bitmap("spritesheet.png");
    if (!bg_home || !bg_rank || !bg_play || !spr_items) return false;

    const int C1X = 7, C1Y = 150, CWH = 26;
    const int C2X = 36, C2Y = 150;
    const int C3X = 62, C3Y = 150;
    const int P0X = 90, P0Y = 1, PWH = 35;
    const int P1Y = 39, P1WH = 34;
    const int P2Y = 75;
    const int P3Y = 110;
    const int BEAN_X = 10, BEAN_Y = 179, BEAN_W = 17, BEAN_H = 18;

    icon_coffee_1 = al_create_sub_bitmap(spr_items, C1X, C1Y, CWH, CWH);
    icon_coffee_2 = al_create_sub_bitmap(spr_items, C2X, C2Y, CWH, CWH);
    icon_coffee_3 = al_create_sub_bitmap(spr_items, C3X, C3Y, CWH, CWH);
    icon_sleeping = al_create_sub_bitmap(spr_items, P0X, P0Y, PWH, PWH);
    icon_people1 = al_create_sub_bitmap(spr_items, P0X, P1Y, P1WH, P1WH);
    icon_people2 = al_create_sub_bitmap(spr_items, P0X, P2Y, P1WH, P1WH);
    icon_people3 = al_create_sub_bitmap(spr_items, P0X, P3Y, P1WH, P1WH);
    icon_coffee_bean = al_create_sub_bitmap(spr_items, BEAN_X, BEAN_Y, BEAN_W, BEAN_H);

    return true;
}

void assets_unload(void) {
    if (icon_coffee_1) al_destroy_bitmap(icon_coffee_1);
    if (icon_coffee_2) al_destroy_bitmap(icon_coffee_2);
    if (icon_coffee_3) al_destroy_bitmap(icon_coffee_3);
    if (icon_people1) al_destroy_bitmap(icon_people1);
    if (icon_people2) al_destroy_bitmap(icon_people2);
    if (icon_people3) al_destroy_bitmap(icon_people3);
    if (icon_sleeping) al_destroy_bitmap(icon_sleeping);
    if (icon_coffee_bean) al_destroy_bitmap(icon_coffee_bean);
    if (spr_items) al_destroy_bitmap(spr_items);

    if (bg_home) al_destroy_bitmap(bg_home);
    if (bg_rank) al_destroy_bitmap(bg_rank);
    if (bg_play) al_destroy_bitmap(bg_play);
    if (font_title) al_destroy_font(font_title);
    if (font_ui) al_destroy_font(font_ui);

    spr_items = bg_home = bg_rank = bg_play = NULL;
    icon_coffee_1 = icon_coffee_2 = icon_coffee_3 = NULL;
    icon_sleeping = icon_people1 = icon_people2 = icon_people3 = NULL;
    icon_coffee_bean = NULL;
    font_title = font_ui = NULL;
}

// ================ score.c ================
#define _CRT_SECURE_NO_WARNINGS
#include "score.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Entry s_highscores[MAX_SCORE];
static int   s_count = 0;

static int cmp_asc(const void* a, const void* b) {
    const Entry* A = (const Entry*)a;
    const Entry* B = (const Entry*)b;
    return A->score - B->score;
}

void score_load(const char* path) {
    s_count = 0;
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    char line[256];
    while (s_count < MAX_SCORE && fgets(line, sizeof line, f)) {
        Entry e; e.name[0] = '\0'; e.score = 0;
        if (sscanf_s(line, " %31[^,],%d", e.name, (unsigned)NAME_MAX, &e.score) == 2) {
            s_highscores[s_count++] = e;
        }
    }
    fclose(f);
    qsort(s_highscores, s_count, sizeof(Entry), cmp_asc);
}

void score_add_and_save(int s, const char* name, const char* path) {
    Entry e; e.score = s;
    if (!name || !name[0]) strncpy(e.name, "PLAYER", NAME_MAX);
    else strncpy(e.name, name, NAME_MAX);
    e.name[NAME_MAX - 1] = '\0';

    if (s_count < MAX_SCORE) s_highscores[s_count++] = e;
    else s_highscores[s_count - 1] = e;

    qsort(s_highscores, s_count, sizeof(Entry), cmp_asc);

    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;
    for (int i = 0; i < s_count && i < MAX_SCORE; ++i) {
        fprintf(f, "%s,%d\n", s_highscores[i].name, s_highscores[i].score);
    }
    fclose(f);
}

int score_count_get(void) { return s_count; }

Entry score_get(int index) {
    Entry e; e.score = 0; e.name[0] = '\0';
    if (index >= 0 && index < s_count) return s_highscores[index];
    return e;
}

// ================ screens.c ================
#include <stdio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

#include "screens.h"
#include "assets.h"
#include "score.h"
#include "game.h"

static void fmt_time_s(int sec, char* out, size_t n) {
    if (sec < 0) sec = 0;
    int m = sec / 60;
    int s = sec % 60;
    snprintf(out, n, "%02d:%02d", m, s);
}

static void draw_bg(ALLEGRO_BITMAP* bmp, int W, int H) {
    if (!bmp) { al_clear_to_color(al_map_rgb(18, 18, 24)); return; }
    int bw = al_get_bitmap_width(bmp);
    int bh = al_get_bitmap_height(bmp);
    float scale = (float)W / bw;
    if (bh * scale < H) scale = (float)H / bh;
    float dw = bw * scale, dh = bh * scale;
    float dx = (W - dw) * 0.5f, dy = (H - dh) * 0.5f;
    al_draw_scaled_bitmap(bmp, 0, 0, bw, bh, dx, dy, dw, dh, 0);
}

static bool point_in_rect(float px, float py, Rect r) {
    return (px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h);
}

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank, float mx, float my) {
    draw_bg(bg_home ? bg_home : bg_play, W, H);

    bool hover_start = point_in_rect(mx, my, btn_start);
    bool hover_howto = point_in_rect(mx, my, btn_howto);
    bool hover_rank = point_in_rect(mx, my, btn_rank);

    ALLEGRO_COLOR fill_start = hover_start ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR fill_howto = hover_howto ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR fill_rank = hover_rank ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR text = al_map_rgb(255, 255, 255);

    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 120, ALLEGRO_ALIGN_CENTER, "CLASS 7 : Sleeping Defence");

    al_draw_filled_rounded_rectangle(btn_start.x, btn_start.y, btn_start.x + btn_start.w, btn_start.y + btn_start.h, 6, 6, fill_start);
    al_draw_text(font_ui, text, W / 2, btn_start.y + 15, ALLEGRO_ALIGN_CENTER, "START GAME");

    al_draw_filled_rounded_rectangle(btn_howto.x, btn_howto.y, btn_howto.x + btn_howto.w, btn_howto.y + btn_howto.h, 6, 6, fill_howto);
    al_draw_text(font_ui, text, W / 2, btn_howto.y + 15, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");

    al_draw_filled_rounded_rectangle(btn_rank.x, btn_rank.y, btn_rank.x + btn_rank.w, btn_rank.y + btn_rank.h, 6, 6, fill_rank);
    al_draw_text(font_ui, text, W / 2, btn_rank.y + 15, ALLEGRO_ALIGN_CENTER, "RANKING");

    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "Click the button to start");
}

void draw_play_with_game(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, bool show_ranges) {
    draw_bg(bg_play ? bg_play : bg_home, W, H);

    char t[32]; fmt_time_s(score_second, t, sizeof t);
    GameState gs = game_get_state();

    // 상단 UI 슬롯 그리기
    const float pad = 35.0f;
    const float size = 62.0f;
    const float gap = 18.0f;
    const float inset = 6.0f;
    const float bean_draw_size = 20.0f;

    for (int i = 0; i < 3; ++i) {
        float x1 = pad + i * (size + gap);
        float y1 = pad;
        float x2 = x1 + size, y2 = y1 + size;

        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(60, 60, 60));

        ALLEGRO_BITMAP* icon = (i == 0) ? icon_coffee_1 : (i == 1) ? icon_coffee_2 : icon_coffee_3;
        if (icon) {
            float iw = (float)al_get_bitmap_width(icon);
            float ih = (float)al_get_bitmap_height(icon);
            float dw = size - inset * 2.0f;
            float dh = dw * (ih / iw);
            if (dh > size - inset * 2.0f) { dh = size - inset * 2.0f; dw = dh * (iw / ih); }
            float dx = x1 + (size - dw) * 0.5f;
            float dy = y1 + (size - dh) * 0.5f;
            al_draw_scaled_bitmap(icon, 0, 0, (int)iw, (int)ih, dx, dy, dw, dh, 0);
        }

        bool picked = (selected_item == (i + 1));
        ALLEGRO_COLOR o = picked ? al_map_rgb(255, 215, 0) : al_map_rgb(255, 255, 255);
        al_draw_rectangle(x1 - 1.5f, y1 - 1.5f, x2 + 1.5f, y2 + 1.5f, o, 3.0f);

        if (icon_coffee_bean) {
            float slot_center = x1 + size * 0.5f;
            float bean_y = y2 + 6.0f;
            float bean_w = (float)al_get_bitmap_width(icon_coffee_bean);
            float bean_h = (float)al_get_bitmap_height(icon_coffee_bean);
            float bean_x = slot_center - (bean_draw_size + 20) * 0.5f;

            al_draw_scaled_bitmap(icon_coffee_bean, 0, 0, (int)bean_w, (int)bean_h,
                bean_x, bean_y, bean_draw_size, bean_draw_size, 0);

            int cost = (i == 0) ? 100 : (i == 1) ? 75 : 50;
            al_draw_textf(font_ui, al_map_rgb(0, 0, 0), bean_x + bean_draw_size + 7, bean_y + 7, 0, "%d", cost);
        }
    }

    // 게임 상태 UI
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 140, ALLEGRO_ALIGN_CENTER, "SLEEPING DEFENCE");
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), 10, 160, 0, "TIME: %s | Caffeine: %d | Lives: %d", t, gs.caffeine, gs.lives);
    al_draw_textf(font_ui, al_map_rgb(200, 220, 240), 10, 180, 0, "Stage %d/%d | Kills: %d/%d", gs.stage, MAX_STAGES, gs.stage_kills, KILLS_TO_ADVANCE);

    // 조작법 안내
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, H - 100, ALLEGRO_ALIGN_CENTER, "WASD: Select Item | Arrow: Move Cursor | Space: Place/Sell | R: Show Ranges");
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "Enter: Force Win | Backspace: Force Lose | ESC: Quit");

    // 게임 그리드 그리기
    game_draw_grid(W, H, sel_col, sel_row, show_ranges);
}

void draw_play(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, const int marks[GRID_ROWS][GRID_COLS]) {
    // 호환성을 위한 래퍼 함수 - 실제로는 새로운 게임 로직 사용
    draw_play_with_game(W, H, score_second, sel_col, sel_row, selected_item, false);
}

void draw_howto(int W, int H) {
    draw_bg(bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 120, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");

    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 180, ALLEGRO_ALIGN_CENTER, "Defend against sleeping students!");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 210, ALLEGRO_ALIGN_CENTER, "Place coffee towers to wake them up");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 240, ALLEGRO_ALIGN_CENTER, "Resource towers generate caffeine");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 270, ALLEGRO_ALIGN_CENTER, "Tank towers absorb damage");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 300, ALLEGRO_ALIGN_CENTER, "Survive all 5 stages to win!");

    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");
}

void draw_rank(int W, int H) {
    draw_bg(bg_rank ? bg_rank : bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 120, ALLEGRO_ALIGN_CENTER, "RANKING");

    int view = score_count_get(); if (view > 10) view = 10;
    for (int i = 0; i < view; ++i) {
        Entry e = score_get(i);
        char t[32]; fmt_time_s(e.score, t, sizeof t);
        al_draw_textf(font_ui, al_map_rgb(255, 255, 255),
            W / 2, 140 + (i + 1) * 30, ALLEGRO_ALIGN_CENTER,
            "%d. %s : %s", i + 1, e.name, t);
    }
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 55, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");
}

void draw_end(int W, int H, const char* name_buf, int score_second, bool success) {
    draw_bg(bg_play, W, H);

    char t[32]; fmt_time_s(score_second, t, sizeof t);
    const char* msg = success ? "SUCCESS!" : "FAIL!";
    ALLEGRO_COLOR c = success ? al_map_rgb(120, 220, 120) : al_map_rgb(230, 120, 120);

    al_draw_text(font_title, c, W / 2, 120, ALLEGRO_ALIGN_CENTER, msg);
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), W / 2, 160, ALLEGRO_ALIGN_CENTER, "Your time : %s", t);
    al_draw_text(font_ui, al_map_rgb(255, 255, 255), W / 2, 210, ALLEGRO_ALIGN_CENTER, "Enter your name and press ENTER");

    char line[64];
    snprintf(line, sizeof line, "[ %s_ ]", (name_buf ? name_buf : ""));
    al_draw_text(font_ui, al_map_rgb(200, 230, 255), W / 2, 240, ALLEGRO_ALIGN_CENTER, line);
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, 300, ALLEGRO_ALIGN_CENTER, "ESC to quit");
}

// ================ game.c ================
#include "game.h"
#include "assets.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

// 그리드 레이아웃
#define CELL_W 80
#define CELL_H 67
#define GRID_X 280
#define GRID_Y 240

static Tower grid[GRID_ROWS][GRID_COLS];
static Enemy enemies[MAX_ENEMIES];
static GameState game_state;
static double last_enemy_spawn_time = 0.0;

static inline int tower_max_hp(TowerType t) {
    if (t == TOWER_ATTACK) return ATTACK_TOWER_HP;
    if (t == TOWER_RESOURCE) return RESOURCE_TOWER_HP;
    if (t == TOWER_TANK) return TANK_TOWER_HP;
    return 0;
}

static void cell_rect(int row, int col, float* x1, float* y1, float* x2, float* y2) {
    *x1 = GRID_X + col * CELL_W;
    *y1 = GRID_Y + row * CELL_H;
    *x2 = *x1 + CELL_W;
    *y2 = *y1 + CELL_H;
}

static float distance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void game_init(void) {
    srand(time(NULL));

    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c].type = TOWER_EMPTY;
            grid[r][c].cooldown = 0.0f;
            grid[r][c].hp = 0;
        }
    }
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
        enemies[i].atk_cooldown = 0.0f;
    }
    game_state.caffeine = 200;
    game_state.lives = 10;
    game_state.stage = 1;
    game_state.stage_kills = 0;
    game_state.cleared = false;
    game_state.game_over = false;

    last_enemy_spawn_time = al_get_time();
}

void game_reset(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
    }
    game_state.game_over = false;
    game_state.cleared = false;
}

static void spawn_enemy() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].x = GRID_X + GRID_COLS * CELL_W + 20; // 오른쪽에서 시작
            enemies[i].y = GRID_Y + (rand() % (int)(GRID_ROWS * CELL_H));
            enemies[i].hp = ENEMY_HP;
            enemies[i].atk_cooldown = 0.0f;
            break;
        }
    }
}

static void on_enemy_killed(void) {
    game_state.stage_kills++;
    if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
        // 모든 적 제거
        for (int i = 0; i < MAX_ENEMIES; ++i) {
            enemies[i].active = false;
        }

        if (game_state.stage >= MAX_STAGES) {
            game_state.cleared = true;
            return;
        }

        game_state.stage++;
        game_state.stage_kills = 0;
        last_enemy_spawn_time = al_get_time();
    }
}

void game_update(float dt) {
    if (game_state.cleared || game_state.game_over) return;

    // 타워 로직
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float tower_cx = GRID_X + c * CELL_W + CELL_W / 2.0f;
            float tower_cy = GRID_Y + r * CELL_H + CELL_H / 2.0f;

            if (grid[r][c].cooldown > 0) {
                grid[r][c].cooldown -= dt;
            }

            if (grid[r][c].type == TOWER_ATTACK && grid[r][c].cooldown <= 0) {
                Enemy* target = NULL;
                float min_dist = ATTACK_TOWER_RANGE;

                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (enemies[i].active) {
                        float d = distance(tower_cx, tower_cy, enemies[i].x, enemies[i].y);
                        if (d < min_dist) {
                            min_dist = d;
                            target = &enemies[i];
                        }
                    }
                }

                if (target) {
                    target->hp -= ATTACK_TOWER_DAMAGE;
                    if (target->hp <= 0) {
                        target->active = false;
                        game_state.caffeine += 20;
                        on_enemy_killed();
                    }
                    grid[r][c].cooldown = ATTACK_TOWER_COOLDOWN;
                }
            }
            else if (grid[r][c].type == TOWER_RESOURCE && grid[r][c].cooldown <= 0) {
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                grid[r][c].cooldown = RESOURCE_TOWER_COOLDOWN;
            }
        }
    }

    // 적 로직
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (enemies[i].atk_cooldown > 0.0f) {
            enemies[i].atk_cooldown -= dt;
        }

        Tower* target_cell = NULL;
        float best_d = ENEMY_ATTACK_RANGE;

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_EMPTY) continue;
                float x1, y1, x2, y2;
                cell_rect(r, c, &x1, &y1, &x2, &y2);
                float tcx = (x1 + x2) * 0.5f;
                float tcy = (y1 + y2) * 0.5f;
                float d = distance(enemies[i].x, enemies[i].y, tcx, tcy);
                if (d < best_d) {
                    best_d = d;
                    target_cell = &grid[r][c];
                }
            }
        }

        if (target_cell != NULL) {
            // 공격
            if (enemies[i].atk_cooldown <= 0.0f) {
                target_cell->hp -= ENEMY_ATTACK_DAMAGE;
                enemies[i].atk_cooldown = ENEMY_ATTACK_COOLDOWN;

                if (target_cell->hp <= 0) {
                    target_cell->type = TOWER_EMPTY;
                    target_cell->cooldown = 0.0f;
                    target_cell->hp = 0;
                }
            }
        }
        else {
            // 이동
            enemies[i].x -= ENEMY_SPEED;
            if (enemies[i].x <= GRID_X - 20) {
                enemies[i].active = false;
                game_state.lives--;
                if (game_state.lives <= 0) {
                    game_state.game_over = true;
                }
            }
        }
    }

    // 적 스폰
    if (game_state.stage_kills < KILLS_TO_ADVANCE &&
        (al_get_time() - last_enemy_spawn_time > 2.0)) {
        spawn_enemy();
        last_enemy_spawn_time = al_get_time();
    }
}

void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges) {
    // 그리드 셀 및 타워 그리기
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float x1, y1, x2, y2;
            cell_rect(r, c, &x1, &y1, &x2, &y2);

            // 그리드 라인
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(100, 100, 100), 1.0f);

            if (grid[r][c].type != TOWER_EMPTY) {
                float cx = (x1 + x2) * 0.5f;
                float cy = (y1 + y2) * 0.5f;

                // 타워 스프라이트 그리기 (MJ의 에셋 사용)
                ALLEGRO_BITMAP* tower_sprite = NULL;
                if (grid[r][c].type == TOWER_ATTACK) tower_sprite = icon_people1;
                else if (grid[r][c].type == TOWER_RESOURCE) tower_sprite = icon_people2;
                else if (grid[r][c].type == TOWER_TANK) tower_sprite = icon_people3;

                if (tower_sprite) {
                    float sw = al_get_bitmap_width(tower_sprite);
                    float sh = al_get_bitmap_height(tower_sprite);
                    float scale = (CELL_W < CELL_H ? CELL_W : CELL_H) * 0.8f / (sw > sh ? sw : sh);
                    float dw = sw * scale, dh = sh * scale;
                    al_draw_scaled_bitmap(tower_sprite, 0, 0, sw, sh,
                        cx - dw / 2, cy - dh / 2, dw, dh, 0);
                }

                // HP 바
                int maxhp = tower_max_hp(grid[r][c].type);
                if (maxhp > 0) {
                    float ratio = (float)grid[r][c].hp / (float)maxhp;
                    float barw = CELL_W - 10.0f;
                    float bx = x1 + 5.0f;
                    float by = y1 + 5.0f;
                    al_draw_filled_rectangle(bx, by, bx + barw, by + 4, al_map_rgb(40, 40, 40));
                    al_draw_filled_rectangle(bx, by, bx + barw * ratio, by + 4, al_map_rgb(220, 120, 120));
                }
            }
        }
    }

    // 적 그리기
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].active) {
            // 적 스프라이트 (졸고 있는 학생)
            if (icon_sleeping) {
                float sw = al_get_bitmap_width(icon_sleeping);
                float sh = al_get_bitmap_height(icon_sleeping);
                float scale = 0.8f;
                float dw = sw * scale, dh = sh * scale;
                al_draw_scaled_bitmap(icon_sleeping, 0, 0, sw, sh,
                    enemies[i].x - dw / 2, enemies[i].y - dh / 2, dw, dh, 0);
            }
            else {
                // 폴백: 사각형
                al_draw_filled_rectangle(
                    enemies[i].x - 8, enemies[i].y - 8,
                    enemies[i].x + 8, enemies[i].y + 8,
                    al_map_rgb(150, 50, 200)
                );
            }

            // 적 HP 바
            float eratio = (float)enemies[i].hp / (float)ENEMY_HP;
            if (eratio < 0.0f) eratio = 0.0f;
            float ex1 = enemies[i].x - 12, ey1 = enemies[i].y - 20;
            float ex2 = enemies[i].x + 12, ey2 = ey1 + 4;
            al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
            al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * eratio, ey2, al_map_rgb(120, 220, 120));
        }
    }

    // 공격 범위 표시
    if (show_ranges) {
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_ATTACK) {
                    float x1, y1, x2, y2;
                    cell_rect(r, c, &x1, &y1, &x2, &y2);
                    float cx = (x1 + x2) * 0.5f;
                    float cy = (y1 + y2) * 0.5f;
                    al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 120), 2.0f);
                }
            }
        }
    }

    // 선택된 셀의 공격 범위
    if (!show_ranges && grid[cursor_row][cursor_col].type == TOWER_ATTACK) {
        float x1, y1, x2, y2;
        cell_rect(cursor_row, cursor_col, &x1, &y1, &x2, &y2);
        float cx = (x1 + x2) * 0.5f;
        float cy = (y1 + y2) * 0.5f;
        al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 150), 2.0f);
    }

    // 커서 표시
    if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
        float cx1, cy1, cx2, cy2;
        cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
        al_draw_rectangle(cx1 + 2, cy1 + 2, cx2 - 2, cy2 - 2, al_map_rgb(255, 255, 0), 3.0f);
    }
}

void game_place_tower(TowerType type, int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    if (grid[row][col].type != TOWER_EMPTY) return;

    int cost = 0;
    if (type == TOWER_ATTACK) cost = ATTACK_TOWER_COST;
    else if (type == TOWER_RESOURCE) cost = RESOURCE_TOWER_COST;
    else if (type == TOWER_TANK) cost = TANK_TOWER_COST;

    if (game_state.caffeine >= cost) {
        game_state.caffeine -= cost;
        grid[row][col].type = type;
        grid[row][col].cooldown = 0;
        grid[row][col].hp = tower_max_hp(type);
    }
}

void game_sell_tower(int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    Tower* cell = &grid[row][col];
    if (cell->type == TOWER_EMPTY) return;

    int refund = 0;
    if (cell->type == TOWER_ATTACK) refund = ATTACK_TOWER_COST / 2;
    else if (cell->type == TOWER_RESOURCE) refund = RESOURCE_TOWER_COST / 2;
    else if (cell->type == TOWER_TANK) refund = TANK_TOWER_COST / 2;

    game_state.caffeine += refund;
    cell->type = TOWER_EMPTY;
    cell->hp = 0;
    cell->cooldown = 0;
}

GameState game_get_state(void) {
    return game_state;
}
#endif

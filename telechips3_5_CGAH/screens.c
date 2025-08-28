// screens.c
#include <stdio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

#include "screens.h"
#include "assets.h"
#include "score.h"
#include "game.h"

// 시간(초) → "MM:SS"
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

    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 150, ALLEGRO_ALIGN_CENTER, "CLASS 7 : Sleeping Defence");

    al_draw_filled_rounded_rectangle(btn_start.x, btn_start.y, btn_start.x + btn_start.w, btn_start.y + btn_start.h, 6, 6, fill_start);
    al_draw_text(font_ui, al_map_rgb(255, 255, 255), W / 2, btn_start.y + 15, ALLEGRO_ALIGN_CENTER, "START GAME");

    al_draw_filled_rounded_rectangle(btn_howto.x, btn_howto.y, btn_howto.x + btn_howto.w, btn_howto.y + btn_howto.h, 6, 6, fill_howto);
    al_draw_text(font_ui, al_map_rgb(255, 255, 255), W / 2, btn_howto.y + 15, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");

    al_draw_filled_rounded_rectangle(btn_rank.x, btn_rank.y, btn_rank.x + btn_rank.w, btn_rank.y + btn_rank.h, 6, 6, fill_rank);
    al_draw_text(font_ui, al_map_rgb(255, 255, 255), W / 2, btn_rank.y + 15, ALLEGRO_ALIGN_CENTER, "RANKING");

    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "Click the button to start");
}

void draw_play_with_game(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, bool show_ranges) {
    draw_bg(bg_play ? bg_play : bg_home, W, H);

    char t[32]; fmt_time_s(score_second, t, sizeof t);
    GameState gs = game_get_state();

    // ── 상단 슬롯(커피 3종) ─────────────────────────────────────
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

        // 코스트 표시(콩 아이콘 + 숫자)
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

    // ── 라이프 게이지(우상단) ───────────────────────────────────
    int lives = gs.lives;
    if (lives < 0) lives = 0;
    if (lives > 5) lives = 5;

    ALLEGRO_BITMAP* life_bmp = NULL;
    switch (lives) {
    case 5: life_bmp = icon_lifegauge1; break; // 5칸
    case 4: life_bmp = icon_lifegauge2; break; // 4칸
    case 3: life_bmp = icon_lifegauge3; break; // 3칸
    case 2: life_bmp = icon_lifegauge4; break; // 2칸
    case 1: life_bmp = icon_lifegauge5; break; // 1칸
    default: life_bmp = icon_lifegauge6; break; // 0칸
    }

    float gx = (float)W - pad;
    float gy = pad + 8.0f;
    if (life_bmp) {
        float sw = (float)al_get_bitmap_width(life_bmp);
        float sh = (float)al_get_bitmap_height(life_bmp);
        float scale = 3.5f;                 // 보기 좋게 확대
        float dw = sw * scale, dh = sh * scale;
        al_draw_scaled_bitmap(life_bmp, 0, 0, sw, sh, gx - dw, gy, dw, dh, 0);
    }
    else {
        // 폴백 텍스트(스프라이트 없을 때만)
        al_draw_textf(font_ui, al_map_rgb(0, 0, 0), gx - 100, gy, 0, "LIVES: %d", gs.lives);
    }

    // ── 게임 상태 텍스트(라이프 숫자는 표시하지 않음) ───────────
    al_draw_textf(font_ui, al_map_rgb(0, 0, 0), 40, 140, 0,
        "TIME: %s | Caffeine: %d", t, gs.caffeine);
    al_draw_textf(font_ui, al_map_rgb(0, 0, 0), 40, 160, 0,
        "Stage %d/%d | Kills: %d/%d",
        gs.stage, MAX_STAGES, gs.stage_kills, KILLS_TO_ADVANCE);

    // 조작 안내
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, H - 100, ALLEGRO_ALIGN_CENTER,
        "WASD: Select Item | Arrow: Move Cursor | Space: Place/Sell | R: Show Ranges");
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, H - 80, ALLEGRO_ALIGN_CENTER,
        "Enter: Force Win | Backspace: Force Lose | ESC: Quit");

    // ── 그리드/유닛/적 그리기 ───────────────────────────────────
    game_draw_grid(W, H, sel_col, sel_row, show_ranges);
}

// 구 버전 호환용(사용하지 않아도 됨)
void draw_play(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, const int marks[GRID_ROWS][GRID_COLS]) {
    draw_play_with_game(W, H, score_second, sel_col, sel_row, selected_item, false);
}

void draw_howto(int W, int H) {
    draw_bg(bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 150, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");

    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 210, ALLEGRO_ALIGN_CENTER, "Defend against sleeping students!");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 240, ALLEGRO_ALIGN_CENTER, "Place coffee towers to wake them up");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 270, ALLEGRO_ALIGN_CENTER, "Resource towers generate caffeine");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 300, ALLEGRO_ALIGN_CENTER, "Tank towers absorb damage");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 330, ALLEGRO_ALIGN_CENTER, "Survive all 5 stages to win!");

    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");
}

void draw_rank(int W, int H) {
    draw_bg(bg_rank ? bg_rank : bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 150, ALLEGRO_ALIGN_CENTER, "RANKING");

    int view = score_count_get(); if (view > 10) view = 10;
    for (int i = 0; i < view; ++i) {
        Entry e = score_get(i);
        char t[32]; fmt_time_s(e.score, t, sizeof t);
        al_draw_textf(font_ui, al_map_rgb(255, 255, 255),
            W / 2, 190 + (i + 1) * 30, ALLEGRO_ALIGN_CENTER,
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

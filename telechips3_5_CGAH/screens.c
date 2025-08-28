#include <stdio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

#include "screens.h"
#include "assets.h"
#include "score.h"

#ifndef GRID_COLS
#define GRID_COLS 10
#endif
#ifndef GRID_ROWS
#define GRID_ROWS 6
#endif

// �׸��� ��ġ �Ķ����(�ȼ�)
static const int GRID_MARGIN_X = 16;   // �¿� ����
static const int GRID_TOP = 149;  // ���� ���� Y

// �ð�(��) �� "MM:SS"
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

// �÷��� ȭ��
//  - �»�� ���� 3��(Ŀ�� ������)
//  - ���� ���
//  - "���ʺ��� 4ĭ, �Ʒ��� 5ĭ" ���� ���� ������ ��������Ʈ ǥ��
//    (0: icon_sleeping, 1/2/3: icon_people1/2/3)
//  - ���� ���� �� �׵θ�

void draw_play(int W, int H, int score_second,
    int sel_col, int sel_row,
    int selected_item,
    const int marks[GRID_ROWS][GRID_COLS])
{
    draw_bg(bg_play ? bg_play : bg_home, W, H);

    // �ð� ���ڿ�
    char t[32]; fmt_time_s(score_second, t, sizeof t);

    // (A) �»�� ���� + ������
    // draw_play() �ȿ��� (�»�� ���� + ������ + Ŀ���� + ����)
    // draw_play() �ȿ��� (�»�� ���� + ������ + Ŀ���� + ����)
    const float pad = 35.0f;
    const float size = 62.0f;   // ���� �ڽ� ũ��
    const float gap = 18.0f;    // ���� ����
    const float inset = 6.0f;   // ���� ����
    const float bean_draw_size = 20.0f; // ȭ�鿡 �׸� Ŀ���� ũ��

    for (int i = 0; i < 3; ++i) {
        float x1 = pad + i * (size + gap);
        float y1 = pad;
        float x2 = x1 + size, y2 = y1 + size;

        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(60, 60, 60));

        // Ŀ�� ������
        ALLEGRO_BITMAP* icon =
            (i == 0) ? icon_coffee_1 :
            (i == 1) ? icon_coffee_2 :
            icon_coffee_3;
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

        // ���� �׵θ�
        bool picked = (selected_item == (i + 1));
        ALLEGRO_COLOR o = picked ? al_map_rgb(255, 215, 0) : al_map_rgb(255, 255, 255);
        al_draw_rectangle(x1 - 1.5f, y1 - 1.5f, x2 + 1.5f, y2 + 1.5f, o, 3.0f);

        // Ŀ���� + ����
        if (icon_coffee_bean) {
            float slot_center = x1 + size * 0.5f;
            float bean_y = y2 + 6.0f;

            float bean_w = (float)al_get_bitmap_width(icon_coffee_bean);
            float bean_h = (float)al_get_bitmap_height(icon_coffee_bean);

            float bean_x = slot_center - (bean_draw_size + 20) * 0.5f;

            al_draw_scaled_bitmap(icon_coffee_bean, 0, 0, (int)bean_w, (int)bean_h,
                bean_x, bean_y, bean_draw_size, bean_draw_size, 0);

            // ���� ���� (10, 20, 30)
            int cost = (i == 0) ? 10 : (i == 1) ? 20 : 30;
            al_draw_textf(font_ui, al_map_rgb(0, 0, 0), bean_x + bean_draw_size + 7, bean_y + 7, 0, "%d", cost); // ��¦ ���� �ø���
        }
    }



    // (B) �ȳ� �ؽ�Ʈ
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 140, ALLEGRO_ALIGN_CENTER, "GAME SCREEN");
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), W / 2, 200, ALLEGRO_ALIGN_CENTER, "TIME : %s", t);
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 260, ALLEGRO_ALIGN_CENTER, "Press ENTER for SUCCESS");
    al_draw_text(font_ui, al_map_rgb(240, 200, 200), W / 2, 290, ALLEGRO_ALIGN_CENTER, "Press BACKSPACE for FAIL");
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "ESC to quit");

    // ���� ��ġ ���
    int avail_w = W - GRID_MARGIN_X * 2;
    int avail_h = H - GRID_TOP - 16;
    int cell_w = avail_w / GRID_COLS;
    int cell_h = avail_h / GRID_ROWS;
    int cell = (cell_w < cell_h) ? cell_w : cell_h;
    int grid_x0 = GRID_MARGIN_X + (avail_w - cell * GRID_COLS) / 2;
    int grid_y0 = GRID_TOP + (avail_h - cell * GRID_ROWS) / 2;

    // ���� ������ ��������Ʈ �׸���
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 3; ++c) {
            int id = marks[r][c]; // 0=�ڴ��л�, 1/2/3=��ü ������

            ALLEGRO_BITMAP* spr =
                (id == 0) ? icon_sleeping :
                (id == 1) ? icon_people1 :
                (id == 2) ? icon_people2 :
                icon_people3;

            if (!spr) continue;

            float iw = (float)al_get_bitmap_width(spr);
            float ih = (float)al_get_bitmap_height(spr);

            const float inset = 4.0f;
            float maxw = (float)cell - inset * 2.0f;
            float maxh = (float)cell - inset * 2.0f;

            float dw = maxw, dh = dw * (ih / iw);
            if (dh > maxh) { dh = maxh; dw = dh * (iw / ih); }

            float dx = (float)(grid_x0 + c * cell) + ((float)cell - dw) * 0.5f;
            float dy = (float)(grid_y0 + r * cell) + ((float)cell - dh) * 0.5f;

            al_draw_scaled_bitmap(spr, 0, 0, (int)iw, (int)ih, dx, dy, dw, dh, 0);
        }
    }

    // ���� ���� ĭ �׵θ�
    if (sel_col >= 0 && sel_col < GRID_COLS && sel_row >= 0 && sel_row < GRID_ROWS) {
        float x1 = (float)(grid_x0 + sel_col * cell);
        float y1 = (float)(grid_y0 + sel_row * cell);
        float x2 = x1 + (float)cell;
        float y2 = y1 + (float)cell;
        al_draw_rectangle(x1 + 0.5f, y1 + 0.5f, x2 - 0.5f, y2 - 0.5f, al_map_rgb(255, 255, 255), 3.0f);
    }
}

void draw_howto(int W, int H) {
    draw_bg(bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 120, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 260, ALLEGRO_ALIGN_CENTER, "EAT POTATO");
    al_draw_text(font_ui, al_map_rgb(240, 200, 200), W / 2, 290, ALLEGRO_ALIGN_CENTER, "EAT SWEET POTATO");
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

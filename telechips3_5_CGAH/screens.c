#include <stdio.h>
#include "screens.h"
#include "score.h"
#include "assets.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

static void fmt_time_s(int sec, char* out, size_t n) {              //�� ��ȯ �Լ�
    if (sec < 0) sec = 0;
    int m = sec / 60;
    int s = sec % 60;
    snprintf(out, n, "%02d:%02d", m, s);
}

static void draw_bg(ALLEGRO_BITMAP* bmp, int W, int H) {                    //���ȭ�� ���� 
    if (!bmp) { al_clear_to_color(al_map_rgb(18, 18, 24)); return; }
    int bw = al_get_bitmap_width(bmp);
    int bh = al_get_bitmap_height(bmp);
    float scale = (float)W / bw;
    if (bh * scale < H) scale = (float)H / bh;
    float dw = bw * scale, dh = bh * scale;
    float dx = (W - dw) * 0.5f, dy = (H - dh) * 0.5f;
    al_draw_scaled_bitmap(bmp, 0, 0, bw, bh, dx, dy, dw, dh, 0);
}

static bool point_in_rect(float px, float py, Rect r) {                                     //Ŀ���� ���� r �ȿ� ������ 1 ����
    return (px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h);
}

// ���� ��ġ �Ķ����
static const int GRID_MARGIN_X = 16;   // �¿� ����
static const int GRID_TOP = 149;   // ���� ���� Y

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank,         //����ȭ�鿡 ǥ�õ� �޴� ���� �׸���
    float mx, float my) {
    draw_bg(bg_home ? bg_home : bg_play, W, H);

    bool hover_start = point_in_rect(mx, my, btn_start);                    //1�̳� 0 ����
    bool hover_howto = point_in_rect(mx, my, btn_howto);
    bool hover_rank = point_in_rect(mx, my, btn_rank);

    ALLEGRO_COLOR fill_start = hover_start ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);             //��ư�� Ŀ�� ���ٴ�� ��ư �� (���->�ϴ�)�� �ٲ�
    ALLEGRO_COLOR fill_howto = hover_howto ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR fill_rank = hover_rank ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR text = al_map_rgb(255, 255, 255);                                                                             //���ڻ� : ���

    al_draw_text(font_title, al_map_rgb(20, 20, 20),                                                            //���� Ÿ��Ʋ ����
        W / 2, 120, ALLEGRO_ALIGN_CENTER, "CLASS 7 : Sleeping Defence");

    al_draw_filled_rounded_rectangle(btn_start.x, btn_start.y,                                             //���� ���� ��ư
        btn_start.x + btn_start.w, btn_start.y + btn_start.h, 6, 6, fill_start);
    al_draw_text(font_ui, text, W / 2, btn_start.y + 15, ALLEGRO_ALIGN_CENTER, "START GAME");

    al_draw_filled_rounded_rectangle(btn_howto.x, btn_howto.y,                                      //���� ��� ��ư
        btn_howto.x + btn_howto.w, btn_howto.y + btn_howto.h, 6, 6, fill_howto);
    al_draw_text(font_ui, text, W / 2, btn_howto.y + 15, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");

    al_draw_filled_rounded_rectangle(btn_rank.x, btn_rank.y,                                            //��ŷ Ȯ�� ��ư
        btn_rank.x + btn_rank.w, btn_rank.y + btn_rank.h, 6, 6, fill_rank);
    al_draw_text(font_ui, text, W / 2, btn_rank.y + 15, ALLEGRO_ALIGN_CENTER, "RANKING");

    al_draw_text(font_ui, al_map_rgb(220, 220, 230),                                                    //����ȭ�� �� �Ʒ��� ��ġ�� ����
        W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "Click the button to start");
}

void draw_play(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, const int marks[GRID_ROWS][GRID_COLS]) {                                                                   //���� �÷��� ȭ�鿡 ��µ� ����. ���� ����ְ� ������ textf �Լ� ����ϱ�
    draw_bg(bg_play ? bg_play : bg_home, W, H);
    char t[32];
    fmt_time_s(score_second, t, sizeof t);

    const float pad = 35.0f;   // ȭ�� �����ڸ� ����
    const float size = 30.0f;  // ĭ �� �� ����
    const float gap = 12.0f;   // ĭ ���� ����

    // ���� �� ũ�� ���(���簢��)
    int avail_w = W - GRID_MARGIN_X * 2;
    int avail_h = H - GRID_TOP - 16;
    int cell_w = avail_w / GRID_COLS;
    int cell_h = avail_h / GRID_ROWS;
    int cell = (cell_w < cell_h) ? cell_w : cell_h;

    int grid_x0 = GRID_MARGIN_X + (avail_w - cell * GRID_COLS) / 2;
    int grid_y0 = GRID_TOP + (avail_h - cell * GRID_ROWS) / 2;

    for (int i = 0; i < 3; ++i) {
        float x1 = pad + i * (size + gap);
        float y1 = pad;
        float x2 = x1 + size;
        float y2 = y1 + size;
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgba(255, 255, 255, 40));
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(60, 60, 60));
        // ���� ���ο� ���� �׵θ� ��(���/�����)
        bool picked = (selected_item == (i + 1));
        ALLEGRO_COLOR outline = picked ? al_map_rgb(255, 215, 0) : al_map_rgb(255, 255, 255);
        al_draw_rectangle(x1 - 1.5f, y1 - 1.5f, x2 + 1.5f, y2 + 1.5f, outline, 3.0f);
    }

    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 140, ALLEGRO_ALIGN_CENTER, "GAME SCREEN");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 260, ALLEGRO_ALIGN_CENTER, "Press ENTER for SUCCESS");
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), W / 2, 200, ALLEGRO_ALIGN_CENTER, "TIME : %s", t);
    al_draw_text(font_ui, al_map_rgb(240, 200, 200), W / 2, 290, ALLEGRO_ALIGN_CENTER, "Press BACKSPACE for FAIL");
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "ESC to quit");
    

    // ��ĥ�� ĭ ä���(������ 50%)
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            int id = marks[r][c];           // 0=����, 1/2/3=�� ������
            if (!id) continue;

            ALLEGRO_COLOR fill;
            if (id == 1)      fill = al_map_rgba(255, 0, 0, 128); // ���� 50%
            else if (id == 2) fill = al_map_rgba(0, 128, 255, 128); // �Ķ� 50%
            else               fill = al_map_rgba(255, 215, 0, 128); // ��� 50%

            // ĭ ���ʿ� ���� �ְ� ä���(�׵θ��� �� ��ġ��)
            float x1 = (float)(grid_x0 + c * cell) + 4.0f;
            float y1 = (float)(grid_y0 + r * cell) + 4.0f;
            float x2 = x1 + (float)cell - 8.0f;
            float y2 = y1 + (float)cell - 8.0f;
            al_draw_filled_rectangle(x1, y1, x2, y2, fill);
        }
    }

    //���� ���� �� �׵θ�(���)
    if (sel_col >= 0 && sel_col < GRID_COLS && sel_row >= 0 && sel_row < GRID_ROWS) {
        float x1 = (float)(grid_x0 + sel_col * cell);
        float y1 = (float)(grid_y0 + sel_row * cell);
        float x2 = x1 + (float)cell;
        float y2 = y1 + (float)cell;
        // ���ȼ� �������� �� ���� ����
        al_draw_rectangle(x1 + 0.5f, y1 + 0.5f, x2 - 0.5f, y2 - 0.5f, al_map_rgb(255, 255, 255), 3.0f);
    }
}

void draw_howto(int W, int H) {                                                                 //���� ��� ȭ�鿡 ��µ� ����. ���߿� �������ֱ�
    draw_bg(bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 120, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 260, ALLEGRO_ALIGN_CENTER, "EAT POTATO");
    al_draw_text(font_ui, al_map_rgb(240, 200, 200), W / 2, 290, ALLEGRO_ALIGN_CENTER, "EAT SWEET POTATO");
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");
}

void draw_rank(int W, int H) {                                                      //��ŷ ȭ�鿡 ��µ� ����. 
    draw_bg(bg_rank ? bg_rank : bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 120, ALLEGRO_ALIGN_CENTER, "RANKING");              //���� �ؽ�Ʈ
    int view = score_count_get(); 
    if (view > 10) view = 10;                                 //ǥ���� ��ŷ ��. 10 �ʰ��ϸ� 10���� ������
    for (int i = 0; i < view; ++i) {
        char t[32];
        Entry e = score_get(i);
        fmt_time_s(e.score, t, sizeof t);
        al_draw_textf(font_ui, al_map_rgb(255, 255, 255),       
            W / 2, 140 + (i + 1) * 30, ALLEGRO_ALIGN_CENTER, "%d. %s : %s", i + 1, e.name, t);
    }
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 55, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");      //�Ʒ��ʿ� ��µ� ����
}

void draw_end(int W, int H, const char* name_buf, int score_second, bool success) {
    char t[32];
    draw_bg(bg_play, W, H);
    fmt_time_s(score_second, t, sizeof t);
    const char* msg = success ? "SUCCESS!" : "FAIL!";                                           //���� Ȥ�� ���� �� �ش� ���� ����
    ALLEGRO_COLOR c = success ? al_map_rgb(120, 220, 120) : al_map_rgb(230, 120, 120);      //���� Ȥ�� ���� �� ����� ������ ���� (��ũ�� �Ķ��ε�)

    al_draw_text(font_title, c, W / 2, 120, ALLEGRO_ALIGN_CENTER, msg);                         //���� or ���� ���� ���
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), W / 2, 160, ALLEGRO_ALIGN_CENTER, "Your time : %s", t);              //���� ���
    al_draw_text(font_ui, al_map_rgb(255, 255, 255), W / 2, 210, ALLEGRO_ALIGN_CENTER, "Enter your name and press ENTER");    

    char line[64]; 
    snprintf(line, sizeof line, "[ %s_ ]", (name_buf ? name_buf : ""));                             //�г��� �Է� â. �Է¶� �� ���� _ �ٿ��� ���ü� �ø�. 
    al_draw_text(font_ui, al_map_rgb(200, 230, 255), W / 2, 240, ALLEGRO_ALIGN_CENTER, line);           //�г��� �Է�â ���
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, 300, ALLEGRO_ALIGN_CENTER, "ESC to quit");
}

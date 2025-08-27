#include <stdio.h>
#include "screens.h"
#include "score.h"
#include "assets.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

static void fmt_time_s(int sec, char* out, size_t n) {              //초 변환 함수
    if (sec < 0) sec = 0;
    int m = sec / 60;
    int s = sec % 60;
    snprintf(out, n, "%02d:%02d", m, s);
}

static void draw_bg(ALLEGRO_BITMAP* bmp, int W, int H) {                    //배경화면 설정 
    if (!bmp) { al_clear_to_color(al_map_rgb(18, 18, 24)); return; }
    int bw = al_get_bitmap_width(bmp);
    int bh = al_get_bitmap_height(bmp);
    float scale = (float)W / bw;
    if (bh * scale < H) scale = (float)H / bh;
    float dw = bw * scale, dh = bh * scale;
    float dx = (W - dw) * 0.5f, dy = (H - dh) * 0.5f;
    al_draw_scaled_bitmap(bmp, 0, 0, bw, bh, dx, dy, dw, dh, 0);
}

static bool point_in_rect(float px, float py, Rect r) {                                     //커서가 범위 r 안에 들어오면 1 리턴
    return (px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h);
}

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank,         //메인화면에 표시될 메뉴 상자 그리기
    float mx, float my) {
    draw_bg(bg_home ? bg_home : bg_play, W, H);

    bool hover_start = point_in_rect(mx, my, btn_start);                    //1이나 0 가짐
    bool hover_howto = point_in_rect(mx, my, btn_howto);
    bool hover_rank = point_in_rect(mx, my, btn_rank);

    ALLEGRO_COLOR fill_start = hover_start ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);             //버튼에 커서 갖다대면 버튼 색 (노랑->하늘)로 바뀜
    ALLEGRO_COLOR fill_howto = hover_howto ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR fill_rank = hover_rank ? al_map_rgb(153, 217, 234) : al_map_rgb(255, 207, 106);
    ALLEGRO_COLOR text = al_map_rgb(255, 255, 255);                                                                             //글자색 : 흰색

    al_draw_text(font_title, al_map_rgb(20, 20, 20),                                                            //게임 타이틀 설정
        W / 2, 120, ALLEGRO_ALIGN_CENTER, "CLASS 7 : Sleeping Defence");

    al_draw_filled_rounded_rectangle(btn_start.x, btn_start.y,                                             //게임 시작 버튼
        btn_start.x + btn_start.w, btn_start.y + btn_start.h, 6, 6, fill_start);
    al_draw_text(font_ui, text, W / 2, btn_start.y + 15, ALLEGRO_ALIGN_CENTER, "START GAME");

    al_draw_filled_rounded_rectangle(btn_howto.x, btn_howto.y,                                      //게임 방법 버튼
        btn_howto.x + btn_howto.w, btn_howto.y + btn_howto.h, 6, 6, fill_howto);
    al_draw_text(font_ui, text, W / 2, btn_howto.y + 15, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");

    al_draw_filled_rounded_rectangle(btn_rank.x, btn_rank.y,                                            //랭킹 확인 버튼
        btn_rank.x + btn_rank.w, btn_rank.y + btn_rank.h, 6, 6, fill_rank);
    al_draw_text(font_ui, text, W / 2, btn_rank.y + 15, ALLEGRO_ALIGN_CENTER, "RANKING");

    al_draw_text(font_ui, al_map_rgb(220, 220, 230),                                                    //메인화면 맨 아래에 위치한 문구
        W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "Click the button to start");
}

void draw_play(int W, int H, int score_second) {                                                                   //게임 플레이 화면에 출력될 문구. 변수 집어넣고 싶으면 textf 함수 사용하기
    draw_bg(bg_play ? bg_play : bg_home, W, H);
    char t[32];
    fmt_time_s(score_second, t, sizeof t);
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 140, ALLEGRO_ALIGN_CENTER, "GAME SCREEN");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 260, ALLEGRO_ALIGN_CENTER, "Press SPACEBAR and ENTER for SUCCESS");
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), W / 2, 200, ALLEGRO_ALIGN_CENTER, "TIME : %s", t);
    al_draw_text(font_ui, al_map_rgb(240, 200, 200), W / 2, 290, ALLEGRO_ALIGN_CENTER, "Press BACKSPACE for FAIL");
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "ESC to quit");
}

void draw_howto(int W, int H) {                                                                 //게임 방법 화면에 출력될 문구. 나중에 변경해주기
    draw_bg(bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(255, 255, 255), W / 2, 120, ALLEGRO_ALIGN_CENTER, "HOW TO PLAY");
    al_draw_text(font_ui, al_map_rgb(200, 220, 240), W / 2, 260, ALLEGRO_ALIGN_CENTER, "EAT POTATO");
    al_draw_text(font_ui, al_map_rgb(240, 200, 200), W / 2, 290, ALLEGRO_ALIGN_CENTER, "EAT SWEET POTATO");
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 80, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");
}

void draw_rank(int W, int H) {                                                      //랭킹 화면에 출력될 문구. 
    draw_bg(bg_rank ? bg_rank : bg_play, W, H);
    al_draw_text(font_title, al_map_rgb(20, 20, 20), W / 2, 120, ALLEGRO_ALIGN_CENTER, "RANKING");              //제목 텍스트
    int view = score_count_get(); 
    if (view > 10) view = 10;                                 //표시할 랭킹 수. 10 초과하면 10개로 제한함
    for (int i = 0; i < view; ++i) {
        char t[32];
        Entry e = score_get(i);
        fmt_time_s(e.score, t, sizeof t);
        al_draw_textf(font_ui, al_map_rgb(255, 255, 255),       
            W / 2, 140 + (i + 1) * 30, ALLEGRO_ALIGN_CENTER, "%d. %s : %s", i + 1, e.name, t);
    }
    al_draw_text(font_ui, al_map_rgb(220, 220, 230), W / 2, H - 55, ALLEGRO_ALIGN_CENTER, "back to menu : SPACE BAR");      //아래쪽에 출력될 문구
}

void draw_end(int W, int H, const char* name_buf, int score_second, bool success) {
    char t[32];
    draw_bg(bg_play, W, H);
    fmt_time_s(score_second, t, sizeof t);
    const char* msg = success ? "SUCCESS!" : "FAIL!";                                           //성공 혹은 실패 시 해당 문구 저장
    ALLEGRO_COLOR c = success ? al_map_rgb(120, 220, 120) : al_map_rgb(230, 120, 120);      //성공 혹은 실패 시 출력할 문구의 색깔 (핑크랑 파랑인듯)

    al_draw_text(font_title, c, W / 2, 120, ALLEGRO_ALIGN_CENTER, msg);                         //성공 or 실패 문구 출력
    al_draw_textf(font_ui, al_map_rgb(220, 220, 230), W / 2, 160, ALLEGRO_ALIGN_CENTER, "Your time : %s", t);              //점수 출력
    al_draw_text(font_ui, al_map_rgb(255, 255, 255), W / 2, 210, ALLEGRO_ALIGN_CENTER, "Enter your name and press ENTER");    

    char line[64]; 
    snprintf(line, sizeof line, "[ %s_ ]", (name_buf ? name_buf : ""));                             //닉네임 입력 창. 입력란 맨 끝에 _ 붙여서 가시성 올림. 
    al_draw_text(font_ui, al_map_rgb(200, 230, 255), W / 2, 240, ALLEGRO_ALIGN_CENTER, line);           //닉네임 입력창 출력
    al_draw_text(font_ui, al_map_rgb(180, 180, 200), W / 2, 300, ALLEGRO_ALIGN_CENTER, "ESC to quit");
}

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

typedef enum { STATE_MENU = 0, STATE_PLAY, STATE_HOWTO, STATE_RANK, STATE_END } GameState;
typedef enum { RESULT_NONE = 0, RESULT_SUCCESS, RESULT_FAIL } GameResult;

static GameState  g_state = STATE_MENU;
static GameResult g_result = RESULT_NONE;

static char   name_buf[NAME_MAX] = { 0 };
static int    name_len = 0;
static bool   end_recorded = false;
static double play_start_time = 0.0;

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

    ALLEGRO_TIMER* frame_timer = al_create_timer(1.0 / 60.0); // 60Hz
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
    int   score = 0;

    // 격자/아이템 선택 상태
    int selected_item = 0;          // 0=선택없음, 1/2/3
    int sel_col = 0, sel_row = 0;   // 커서 위치(0,0) 시작
    int grid_marks[GRID_ROWS][GRID_COLS]; // 0=없음, 1/2/3 색

    bool running = true, redraw = true;

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(q, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == frame_timer) redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            mx = ev.mouse.x; my = ev.mouse.y;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (g_state == STATE_MENU) {
                if (point_in_rect(mx, my, btn_start)) {
                    g_state = STATE_PLAY;
                    g_result = RESULT_NONE;
                    score = 0;
                    end_recorded = false;
                    name_len = 0; name_buf[0] = '\0';
                    play_start_time = al_get_time();

                    selected_item = 0;
                    sel_col = 0; sel_row = 0;
                    memset(grid_marks, 0, sizeof grid_marks);
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
            // END에서만 문자 입력(닉네임)
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
                // 숫자키 선택/해제 (상단 숫자+키패드)
                switch (key) {
                case ALLEGRO_KEY_1: case ALLEGRO_KEY_PAD_1: selected_item = 1; break;
                case ALLEGRO_KEY_2: case ALLEGRO_KEY_PAD_2: selected_item = 2; break;
                case ALLEGRO_KEY_3: case ALLEGRO_KEY_PAD_3: selected_item = 3; break;
                case ALLEGRO_KEY_4: case ALLEGRO_KEY_PAD_4: selected_item = 0; break;
                }

                // 방향키 커서 이동
                if (key == ALLEGRO_KEY_LEFT && sel_col > 0)               sel_col--;
                if (key == ALLEGRO_KEY_RIGHT && sel_col < GRID_COLS - 1)   sel_col++;
                if (key == ALLEGRO_KEY_UP && sel_row > 0)               sel_row--;
                if (key == ALLEGRO_KEY_DOWN && sel_row < GRID_ROWS - 1)   sel_row++;

                // 스페이스: 현재 셀 반투명 색칠 (1:빨강, 2:파랑, 3:노랑)
                if (key == ALLEGRO_KEY_SPACE) {
                    if (selected_item >= 1 && selected_item <= 3 &&
                        grid_marks[sel_row][sel_col] == 0) {
                        grid_marks[sel_row][sel_col] = selected_item;
                    }
                }

                // 게임 종료(성공/실패) → 점수는 소요 시간(초)
                if (key == ALLEGRO_KEY_ENTER) {
                    g_result = RESULT_SUCCESS;
                    score = (int)(al_get_time() - play_start_time);
                    g_state = STATE_END;
                }
                else if (key == ALLEGRO_KEY_BACKSPACE) {
                    g_result = RESULT_FAIL;
                    score = (int)(al_get_time() - play_start_time);
                    g_state = STATE_END;
                }
            }
            else if (g_state == STATE_END) {
                if (key == ALLEGRO_KEY_ENTER) {
                    if (!end_recorded) {
                        score_add_and_save(score, name_buf, SCORE_FILE);
                        end_recorded = true;
                    }
                    g_state = STATE_MENU;
                }
                else if (key == ALLEGRO_KEY_SPACE) {
                    g_state = STATE_MENU;
                }
            }
        }

        // === 그리기(이벤트 분기 바깥) ===
        if (redraw && al_is_event_queue_empty(q)) {
            switch (g_state) {
            case STATE_MENU:
                draw_menu(W, H, btn_start, btn_howto, btn_rank, mx, my);
                break;
            case STATE_PLAY: {
                int live_sec = (int)(al_get_time() - play_start_time);
                draw_play(W, H, live_sec, sel_col, sel_row, selected_item, grid_marks);
                break;
            }
            case STATE_HOWTO: draw_howto(W, H); break;
            case STATE_RANK:  draw_rank(W, H);  break;
            case STATE_END:   draw_end(W, H, name_buf, score, (g_result == RESULT_SUCCESS)); break;
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

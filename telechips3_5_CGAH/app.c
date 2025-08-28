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
static int  cursor_col = 0, cursor_row = 0;
static int  selected_item = 0;
static bool show_all_ranges = false;

// ── 일시정지 상태 ──
static bool g_paused = false;
static int  pause_sel = 0; // 0: Resume, 1: Main Menu

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

    const int W = 960, H = 720; // 게임창 크기
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

    // 일시정지 창 버튼
    Rect btn_resume = { W / 2.0f - 160, H / 2.0f - 40, 320, 58 };
    Rect btn_main = { W / 2.0f - 160, H / 2.0f + 32, 320, 58 };

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
                    // ★ 일시정지 중에는 게임 로직 정지
                    if (!g_paused) {
                        game_update(1.0f / 60.0f);
                    }

                    GameState gs = game_get_state();
                    if (gs.game_over || gs.lives <= 0) {
                        g_result = RESULT_FAIL;
                        final_score = (int)(al_get_time() - play_start_time);
                        g_state = STATE_END;
                        g_paused = false; // 혹시 모를 잔여 정지 해제
                    }
                    else if (gs.cleared) {
                        g_result = RESULT_SUCCESS;
                        final_score = (int)(al_get_time() - play_start_time);
                        g_state = STATE_END;
                        g_paused = false;
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            mx = ev.mouse.x; my = ev.mouse.y;

            // ★ 마우스로 일시정지 창 버튼 hover → 선택 이동
            if (g_state == STATE_PLAY && g_paused) {
                if (point_in_rect(mx, my, btn_resume)) pause_sel = 0;
                else if (point_in_rect(mx, my, btn_main)) pause_sel = 1;
            }
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
                    g_paused = false; pause_sel = 0;
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
            else if (g_state == STATE_PLAY && g_paused) {
                // ★ 마우스로 일시정지 버튼 클릭
                if (point_in_rect(mx, my, btn_resume)) {
                    g_paused = false;
                }
                else if (point_in_rect(mx, my, btn_main)) {
                    g_state = STATE_MENU;
                    g_paused = false;
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

                // ── 일시정지 창 On/Off ──
                if (key == ALLEGRO_KEY_BACKSPACE) {
                    g_paused = !g_paused;
                    pause_sel = 0; // 기본 Resume에 포커스
                    continue;
                }

                if (g_paused) {
                    // ★ 일시정지 상태에서의 키 조작
                    if (key == ALLEGRO_KEY_LEFT || key == ALLEGRO_KEY_UP) {
                        pause_sel = (pause_sel + 1) % 2;
                    }
                    else if (key == ALLEGRO_KEY_RIGHT || key == ALLEGRO_KEY_DOWN) {
                        pause_sel = (pause_sel + 1) % 2;
                    }
                    else if (key == ALLEGRO_KEY_ENTER || key == ALLEGRO_KEY_SPACE) {
                        if (pause_sel == 0) {
                            // Resume
                            g_paused = false;
                        }
                        else {
                            // Main Menu
                            g_state = STATE_MENU;
                            g_paused = false;
                        }
                    }
                    // 일시정지 중엔 다른 조작 무시
                    continue;
                }

                // ── 평상시 조작 ──
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
                    if (selected_item == 1)      game_place_tower(TOWER_ATTACK, cursor_row, cursor_col);
                    else if (selected_item == 2) game_place_tower(TOWER_RESOURCE, cursor_row, cursor_col);
                    else if (selected_item == 3) game_place_tower(TOWER_TANK, cursor_row, cursor_col);
                    else                          game_sell_tower(cursor_row, cursor_col);
                }

                // 범위 표시 토글
                if (key == ALLEGRO_KEY_R) show_all_ranges = !show_all_ranges;

                // 강제 종료(테스트) — 필요시 유지
                if (key == ALLEGRO_KEY_ENTER) {
                    g_result = RESULT_SUCCESS;
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

                // ★ 일시정지 오버레이(버튼 하이라이트는 마우스 hover 또는 pause_sel로 처리)
                if (g_paused) {
                    draw_pause_overlay(W, H, btn_resume, btn_main, pause_sel, mx, my);
                }
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

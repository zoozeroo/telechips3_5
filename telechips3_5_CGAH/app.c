#define _CRT_SECURE_NO_WARNINGS
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#include "app.h"
#include "assets.h"
#include "score.h"
#include "screens.h"
#include <string.h>

typedef enum { STATE_MENU = 0, STATE_PLAY, STATE_HOWTO, STATE_RANK, STATE_END } GameState;
typedef enum { RESULT_NONE = 0, RESULT_SUCCESS, RESULT_FAIL } GameResult;

static GameState  g_state = STATE_MENU;
static GameResult g_result = RESULT_NONE;

static char name_buf[NAME_MAX] = { 0 };
static int  name_len = 0;
static bool end_recorded = false;
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

    ALLEGRO_TIMER* frame_timer = al_create_timer(1.0 / 60.0); // 화면/로직용 60Hz
    ALLEGRO_TIMER* sec_timer = al_create_timer(1.0);        // 1Hz(=1초) 타이머
    ALLEGRO_EVENT_QUEUE* q = al_create_event_queue();               //모든 이벤트 큐에 저장

    al_register_event_source(q, al_get_timer_event_source(frame_timer));
    al_register_event_source(q, al_get_timer_event_source(sec_timer));
    al_register_event_source(q, al_get_display_event_source(disp));
    al_register_event_source(q, al_get_keyboard_event_source());
    al_register_event_source(q, al_get_mouse_event_source());
    al_start_timer(frame_timer);
    al_start_timer(sec_timer);

    Rect btn_start = { W / 2.0f - 120, H / 2.0f - 100, 240, 50 };               //버튼 생성
    Rect btn_howto = { W / 2.0f - 120, H / 2.0f - 25, 240, 50 };
    Rect btn_rank = { W / 2.0f - 120, H / 2.0f + 50, 240, 50 };

    float mx = 0, my = 0;                               //마우스 x축, y축 위치
    int score = 0;                                          //게임 점수
    bool running = true, redraw = true;

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(q, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {           //디스플레이 꺼지면 게임 종료
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == frame_timer) {
                redraw = true;
            }
            else if (ev.timer.source == sec_timer) {
                if (g_state == STATE_PLAY) {
                    score += 1; // 1초에 한 번씩 증가
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {         //마우스 축 : 위치
            mx = ev.mouse.x; my = ev.mouse.y;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {      //버튼 클릭하면 실행
            if (g_state == STATE_MENU) {                //메인 메뉴 화면일 때
                if (point_in_rect(mx, my, btn_start)) {         //스타트 버튼을 누르면
                    g_state = STATE_PLAY;                   //플레이 시작
                    g_result = RESULT_NONE;             
                    score = 0;                                      //매 플레이마다 점수 0으로 초기화
                    end_recorded = false;
                    name_len = 0; name_buf[0] = '\0';               //닉네임 0으로 초기화
                    play_start_time = al_get_time();                    //플레이 타임 기록 시작
                }
                else if (point_in_rect(mx, my, btn_howto)) {    //게임방법 버튼을 누르면
                    g_state = STATE_HOWTO;                          //창 넘어가짐
                    g_result = RESULT_NONE; 
                }
                else if (point_in_rect(mx, my, btn_rank)) {     //랭킹 버튼을 누르면
                    g_state = STATE_RANK;                           //랭킹 창으로 넘어감
                    g_result = RESULT_NONE;
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {           //키보드 입력(영어)
            if (g_state == STATE_END) {                                         //게임 종료 시 (닉네임 입력 코드)
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
            if (key == ALLEGRO_KEY_ESCAPE) running = false;                 //esc 누르면 게임 종료

            if (g_state == STATE_HOWTO) {                                               //게임방법 창에서 스페이스바 누르면 메인화면으로 돌아감
                if (key == ALLEGRO_KEY_SPACE) g_state = STATE_MENU;
            }
            else if (g_state == STATE_RANK) {                                           //랭킹 창에서 스페이스바 누르면 메인화면으로 돌아감
                if (key == ALLEGRO_KEY_SPACE) g_state = STATE_MENU;
            }
            else if (g_state == STATE_PLAY) {                                           //플레이 중 엔터를 누르면 성공, 게임 종료 화면으로 넘어감->게임 로직 적용 시 변경될 내용
                if (key == ALLEGRO_KEY_ENTER) {
                    g_result = RESULT_SUCCESS;
                    score = (int)(al_get_time() - play_start_time);
                    g_state = STATE_END;
                }
            }
            else if (key == ALLEGRO_KEY_BACK) {                         //백스페이스 누르면 실패, 게임 종료 화면으로 넘어감
                g_result = RESULT_FAIL;
                score = (int)(al_get_time() - play_start_time);
                g_state = STATE_END;
            }
            else if (g_state == STATE_END) {                                        //게임 종료 화면에서
                if (key == ALLEGRO_KEY_ENTER) {                                 //엔터를 누르면 점수와 닉네임이 저장됨
                    if (!end_recorded) {
                        score_add_and_save(score, name_buf, SCORE_FILE);
                        end_recorded = true;
                    }
                    g_state = STATE_MENU;                                               // 저장 후 메뉴
                }
                else if (key == ALLEGRO_KEY_SPACE) {
                    g_state = STATE_MENU;                                               // 저장 없이 메뉴
                }
            }
        }

        if (redraw && al_is_event_queue_empty(q)) {
            switch (g_state) {
            case STATE_MENU:  draw_menu(W, H, btn_start, btn_howto, btn_rank, mx, my); break;
            case STATE_PLAY: {
                draw_play(W, H, score);           // 실시간 시간 표시
                break;
            }
            case STATE_HOWTO: draw_howto(W, H); break;
            case STATE_RANK:  draw_rank(W, H); break;
            case STATE_END:   draw_end(W, H, name_buf, score, (g_result == RESULT_SUCCESS)); break;
            }
            al_flip_display();
            redraw = false;
        }
    }

    assets_unload();
    al_destroy_event_queue(q);
    al_destroy_timer(sec_timer);
    al_destroy_timer(frame_timer);
    al_destroy_display(disp);
    return 0;
}

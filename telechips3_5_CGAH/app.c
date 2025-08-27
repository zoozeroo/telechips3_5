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

    ALLEGRO_TIMER* frame_timer = al_create_timer(1.0 / 60.0); // ȭ��/������ 60Hz
    ALLEGRO_TIMER* sec_timer = al_create_timer(1.0);        // 1Hz(=1��) Ÿ�̸�
    ALLEGRO_EVENT_QUEUE* q = al_create_event_queue();               //��� �̺�Ʈ ť�� ����

    al_register_event_source(q, al_get_timer_event_source(frame_timer));
    al_register_event_source(q, al_get_timer_event_source(sec_timer));
    al_register_event_source(q, al_get_display_event_source(disp));
    al_register_event_source(q, al_get_keyboard_event_source());
    al_register_event_source(q, al_get_mouse_event_source());
    al_start_timer(frame_timer);
    al_start_timer(sec_timer);

    Rect btn_start = { W / 2.0f - 120, H / 2.0f - 100, 240, 50 };               //��ư ����
    Rect btn_howto = { W / 2.0f - 120, H / 2.0f - 25, 240, 50 };
    Rect btn_rank = { W / 2.0f - 120, H / 2.0f + 50, 240, 50 };

    float mx = 0, my = 0;                               //���콺 x��, y�� ��ġ
    int score = 0;                                          //���� ����
    bool running = true, redraw = true;

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(q, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {           //���÷��� ������ ���� ����
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == frame_timer) {
                redraw = true;
            }
            else if (ev.timer.source == sec_timer) {
                if (g_state == STATE_PLAY) {
                    score += 1; // 1�ʿ� �� ���� ����
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {         //���콺 �� : ��ġ
            mx = ev.mouse.x; my = ev.mouse.y;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {      //��ư Ŭ���ϸ� ����
            if (g_state == STATE_MENU) {                //���� �޴� ȭ���� ��
                if (point_in_rect(mx, my, btn_start)) {         //��ŸƮ ��ư�� ������
                    g_state = STATE_PLAY;                   //�÷��� ����
                    g_result = RESULT_NONE;             
                    score = 0;                                      //�� �÷��̸��� ���� 0���� �ʱ�ȭ
                    end_recorded = false;
                    name_len = 0; name_buf[0] = '\0';               //�г��� 0���� �ʱ�ȭ
                    play_start_time = al_get_time();                    //�÷��� Ÿ�� ��� ����
                }
                else if (point_in_rect(mx, my, btn_howto)) {    //���ӹ�� ��ư�� ������
                    g_state = STATE_HOWTO;                          //â �Ѿ��
                    g_result = RESULT_NONE; 
                }
                else if (point_in_rect(mx, my, btn_rank)) {     //��ŷ ��ư�� ������
                    g_state = STATE_RANK;                           //��ŷ â���� �Ѿ
                    g_result = RESULT_NONE;
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {           //Ű���� �Է�(����)
            if (g_state == STATE_END) {                                         //���� ���� �� (�г��� �Է� �ڵ�)
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
            if (key == ALLEGRO_KEY_ESCAPE) running = false;                 //esc ������ ���� ����

            if (g_state == STATE_HOWTO) {                                               //���ӹ�� â���� �����̽��� ������ ����ȭ������ ���ư�
                if (key == ALLEGRO_KEY_SPACE) g_state = STATE_MENU;
            }
            else if (g_state == STATE_RANK) {                                           //��ŷ â���� �����̽��� ������ ����ȭ������ ���ư�
                if (key == ALLEGRO_KEY_SPACE) g_state = STATE_MENU;
            }
            else if (g_state == STATE_PLAY) {                                           //�÷��� �� ���͸� ������ ����, ���� ���� ȭ������ �Ѿ->���� ���� ���� �� ����� ����
                if (key == ALLEGRO_KEY_ENTER) {
                    g_result = RESULT_SUCCESS;
                    score = (int)(al_get_time() - play_start_time);
                    g_state = STATE_END;
                }
            }
            else if (key == ALLEGRO_KEY_BACK) {                         //�齺���̽� ������ ����, ���� ���� ȭ������ �Ѿ
                g_result = RESULT_FAIL;
                score = (int)(al_get_time() - play_start_time);
                g_state = STATE_END;
            }
            else if (g_state == STATE_END) {                                        //���� ���� ȭ�鿡��
                if (key == ALLEGRO_KEY_ENTER) {                                 //���͸� ������ ������ �г����� �����
                    if (!end_recorded) {
                        score_add_and_save(score, name_buf, SCORE_FILE);
                        end_recorded = true;
                    }
                    g_state = STATE_MENU;                                               // ���� �� �޴�
                }
                else if (key == ALLEGRO_KEY_SPACE) {
                    g_state = STATE_MENU;                                               // ���� ���� �޴�
                }
            }
        }

        if (redraw && al_is_event_queue_empty(q)) {
            switch (g_state) {
            case STATE_MENU:  draw_menu(W, H, btn_start, btn_howto, btn_rank, mx, my); break;
            case STATE_PLAY: {
                draw_play(W, H, score);           // �ǽð� �ð� ǥ��
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

#include "assets.h"
#include <allegro5/allegro_image.h>

ALLEGRO_BITMAP* bg_home = NULL;
ALLEGRO_BITMAP* bg_rank = NULL;
ALLEGRO_BITMAP* bg_play = NULL;

ALLEGRO_FONT* font_title = NULL;
ALLEGRO_FONT* font_ui = NULL;

// ��������Ʈ��Ʈ
ALLEGRO_BITMAP* spr_items = NULL;
ALLEGRO_BITMAP* icon_coffee_1 = NULL;
ALLEGRO_BITMAP* icon_coffee_2 = NULL;
ALLEGRO_BITMAP* icon_coffee_3 = NULL;
ALLEGRO_BITMAP* icon_sleeping = NULL;
ALLEGRO_BITMAP* icon_people1 = NULL;
ALLEGRO_BITMAP* icon_people2 = NULL;
ALLEGRO_BITMAP* icon_people3 = NULL;

bool assets_load(void) {
    font_title = al_create_builtin_font();                      //���� : �⺻ ��Ʈ ���
    font_ui = al_create_builtin_font();                         //UI : �⺻ ��Ʈ ���
    if (!font_title || !font_ui) return false;

    bg_home = al_load_bitmap("background_play.png");                    //����ȭ�� ��� �̹��� -> �ϴ� ���� �÷��� ȭ������ �����ص�
    bg_rank = al_load_bitmap("background_play.png");                        //��ŷȭ�� ��� �̹���
    bg_play = al_load_bitmap("background_play.png");                        //���� �÷��� ��� �̹���
    
    // ��������Ʈ��Ʈ �ε�
    spr_items = al_load_bitmap("spritesheet.png");
    if (!bg_home || !bg_rank || !bg_play || !spr_items) return false;

    const int C1X = 7, C1Y = 150, CWH = 26;         //C : Ŀ��
    const int C2X = 36, C2Y = 150;
    const int C3X = 62, C3Y = 150;
    const int P0X = 90, P0Y = 1, PWH = 35;          // P0 : (���� �ִ�)���
    const int P1Y = 39, P1WH = 34;                      // P1-3 : ������ 1-3���� ���� ����� �����
    const int P2Y = 75;
    const int P3Y = 110;

    icon_coffee_1 = al_create_sub_bitmap(spr_items, C1X, C1Y, CWH, CWH);
    icon_coffee_2 = al_create_sub_bitmap(spr_items, C2X, C2Y, CWH, CWH);
    icon_coffee_3 = al_create_sub_bitmap(spr_items, C3X, C3Y, CWH, CWH);
    icon_sleeping = al_create_sub_bitmap(spr_items, P0X, P0Y, PWH, PWH);
    icon_people1 = al_create_sub_bitmap(spr_items, P0X, P1Y, P1WH, P1WH);
    icon_people2 = al_create_sub_bitmap(spr_items, P0X, P2Y, P1WH, P1WH);
    icon_people3 = al_create_sub_bitmap(spr_items, P0X, P3Y, P1WH, P1WH);

    return true;
}

void assets_unload(void) {                                      //���� â ����
    // sub-bitmap�� �θ𺸴� ���� �ı��ؾ� ��
    if (icon_coffee_1) al_destroy_bitmap(icon_coffee_1);
    if (icon_coffee_2) al_destroy_bitmap(icon_coffee_2);
    if (icon_coffee_3) al_destroy_bitmap(icon_coffee_3);
    if (icon_people1) al_destroy_bitmap(icon_people1);
    if (icon_people2) al_destroy_bitmap(icon_people2);
    if (icon_people3) al_destroy_bitmap(icon_people3);
    if (icon_sleeping) al_destroy_bitmap(icon_sleeping);
    if (spr_items)       al_destroy_bitmap(spr_items);
    
    if (bg_home) al_destroy_bitmap(bg_home);
    if (bg_rank) al_destroy_bitmap(bg_rank);
    if (bg_play) al_destroy_bitmap(bg_play);
    if (font_title) al_destroy_font(font_title);
    if (font_ui)    al_destroy_font(font_ui);

    spr_items = bg_home = bg_rank = bg_play = NULL;
    icon_coffee_1 = icon_coffee_2 = icon_coffee_3 = NULL;
    icon_sleeping = icon_people1 = icon_people2 = icon_people3 = NULL;
    font_title = font_ui = NULL;
}

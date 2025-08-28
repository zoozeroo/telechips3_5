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
ALLEGRO_BITMAP* icon_virus1 = NULL;
ALLEGRO_BITMAP* icon_virus2 = NULL;
ALLEGRO_BITMAP* icon_virus3 = NULL;
ALLEGRO_BITMAP* icon_virus4 = NULL;
ALLEGRO_BITMAP* icon_coffee_bean = NULL;
ALLEGRO_BITMAP* bullet_1 = NULL;
ALLEGRO_BITMAP* bullet_2 = NULL;
ALLEGRO_BITMAP* bullet_3 = NULL;


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
    const int V1X = 7, V1Y = 2;
    const int V2Y = 39;
    const int V3Y = 76;
    const int V4Y = 113;
    const int BEAN_X = 10, BEAN_Y = 179, BEAN_W = 17, BEAN_H = 18;
    const int BW = 9, BH = 9;
    const int B1X = 32, B2X = 44, B3X = 56, BY = 185;

    icon_coffee_1 = al_create_sub_bitmap(spr_items, C1X, C1Y, CWH, CWH);
    icon_coffee_2 = al_create_sub_bitmap(spr_items, C2X, C2Y, CWH, CWH);
    icon_coffee_3 = al_create_sub_bitmap(spr_items, C3X, C3Y, CWH, CWH);
    icon_sleeping = al_create_sub_bitmap(spr_items, P0X, P0Y, PWH, PWH);
    icon_people1 = al_create_sub_bitmap(spr_items, P0X, P1Y, P1WH, P1WH);
    icon_people2 = al_create_sub_bitmap(spr_items, P0X, P2Y, P1WH, P1WH);
    icon_people3 = al_create_sub_bitmap(spr_items, P0X, P3Y, P1WH, P1WH);

    icon_virus1 = al_create_sub_bitmap(spr_items, V1X, V1Y, P1WH, P1WH); // 파
    icon_virus2 = al_create_sub_bitmap(spr_items, V1X, V2Y, P1WH, P1WH); // 노
    icon_virus3 = al_create_sub_bitmap(spr_items, V1X, V3Y, P1WH, P1WH); // 빨
    icon_virus4 = al_create_sub_bitmap(spr_items, V1X, V4Y, P1WH, P1WH); // 하
    
    icon_coffee_bean = al_create_sub_bitmap(spr_items, BEAN_X, BEAN_Y, BEAN_W, BEAN_H);
    
    bullet_1 = al_create_sub_bitmap(spr_items, B1X, BY, BW, BH);
    bullet_2 = al_create_sub_bitmap(spr_items, B2X, BY, BW, BH);
    bullet_3 = al_create_sub_bitmap(spr_items, B3X, BY, BW, BH);


    

    if (!bullet_1 || !bullet_2 || !bullet_3) return false; // 총알 이미지 체크

    return true;
}

// 이미지 해제
void assets_unload(void) {
    if (icon_coffee_1) al_destroy_bitmap(icon_coffee_1);
    if (icon_coffee_2) al_destroy_bitmap(icon_coffee_2);
    if (icon_coffee_3) al_destroy_bitmap(icon_coffee_3);
    if (icon_people1) al_destroy_bitmap(icon_people1);
    if (icon_people2) al_destroy_bitmap(icon_people2);
    if (icon_people3) al_destroy_bitmap(icon_people3);
    if (icon_sleeping) al_destroy_bitmap(icon_sleeping);
    if (icon_virus1) al_destroy_bitmap(icon_virus1);
    if (icon_virus2) al_destroy_bitmap(icon_virus2);
    if (icon_virus3) al_destroy_bitmap(icon_virus3);
    if (icon_virus4) al_destroy_bitmap(icon_virus4);
    if (icon_coffee_bean) al_destroy_bitmap(icon_coffee_bean);
    if (bullet_1) al_destroy_bitmap(bullet_1);
    if (bullet_2) al_destroy_bitmap(bullet_2);
    if (bullet_3) al_destroy_bitmap(bullet_3);
    if (spr_items) al_destroy_bitmap(spr_items);

    if (bg_home) al_destroy_bitmap(bg_home);
    if (bg_rank) al_destroy_bitmap(bg_rank);
    if (bg_play) al_destroy_bitmap(bg_play);
    if (font_title) al_destroy_font(font_title);
    if (font_ui) al_destroy_font(font_ui);

    spr_items = bg_home = bg_rank = bg_play = NULL;
    icon_coffee_1 = icon_coffee_2 = icon_coffee_3 = NULL;
    icon_sleeping = icon_people1 = icon_people2 = icon_people3 = NULL;
    icon_virus1 = icon_virus2 = icon_virus3 = icon_virus4 = NULL;
    icon_coffee_bean = NULL;
    bullet_1 = bullet_2 = bullet_3 = NULL;
    font_title = font_ui = NULL;
}
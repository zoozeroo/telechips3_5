#include "assets.h"
#include <allegro5/allegro_image.h>

//배경화면
ALLEGRO_BITMAP* bg_home = NULL;
ALLEGRO_BITMAP* bg_rank = NULL;
ALLEGRO_BITMAP* bg_play = NULL;
//폰트
ALLEGRO_FONT* font_title = NULL;
ALLEGRO_FONT* font_ui = NULL;
//픽셀 아이콘
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
ALLEGRO_BITMAP* icon_lifegauge1 = NULL;
ALLEGRO_BITMAP* icon_lifegauge2 = NULL;
ALLEGRO_BITMAP* icon_lifegauge3 = NULL;
ALLEGRO_BITMAP* icon_lifegauge4 = NULL;
ALLEGRO_BITMAP* icon_lifegauge5 = NULL;
ALLEGRO_BITMAP* icon_lifegauge6 = NULL;
ALLEGRO_BITMAP* icon_people1_1 = NULL;
ALLEGRO_BITMAP* icon_people2_1 = NULL;
ALLEGRO_BITMAP* icon_people3_1 = NULL;
ALLEGRO_BITMAP* icon_bombeffect1 = NULL;
ALLEGRO_BITMAP* icon_bombeffect2 = NULL;
ALLEGRO_BITMAP* icon_bombeffect3 = NULL;
ALLEGRO_BITMAP* icon_frozen1 = NULL;
ALLEGRO_BITMAP* icon_frozen2 = NULL;
ALLEGRO_BITMAP* icon_frozen3 = NULL;

//폰트, 배경, 아이콘 불러오는 함수
bool assets_load(void) {
    font_title = al_create_builtin_font();
    font_ui = al_create_builtin_font();
    if (!font_title || !font_ui) return false;

    //배경화면 지정. 홈화면, 랭킹화면, 게임 플레이 화면 다 동일하게 지정함
    bg_home = al_load_bitmap("background_play.png");
    bg_rank = al_load_bitmap("background_play.png");
    bg_play = al_load_bitmap("background_play.png");

    spr_items = al_load_bitmap("spritesheet.png");  //스프라이트시트 불러오기
    if (!bg_home || !bg_rank || !bg_play || !spr_items) return false;

    const int C1X = 7, C1Y = 150, CWH = 26;
    const int C2X = 36, C2Y = 150, C3X = 62, C3Y = 150;
    const int P0X = 91, P0Y = 3, PWH = 34;
    const int P1X = 127, P1Y = 39, P2Y = 75, P3Y = 111;
    const int FPX1 = 163, FPX2 = 91, FPY1 = 39, FPY2 = 3, FPWH = 33;
    const int V1X = 7, V1Y = 2, V2Y = 39, V3Y = 76, V4Y = 113;
    const int BEAN_X = 10, BEAN_Y = 179, BEAN_W = 17, BEAN_H = 18;
    const int BW = 9, BH = 9;
    const int B1X = 32, B2X = 44, B3X = 56, BY = 185;
    const int L1X = 89, L2X = 132, L1Y = 152, L2Y = 165, L3Y = 178, LW = 40, LH = 10;
    const int BOMBX1 = 177, BOMBX2 = 171, BOMBX3 = 172,
        BOMBY1 = 74, BOMBY2 = 95, BOMBY3 = 171,
        BWH1 = 18, BWH2 = 29, BWH3 = 26;

    //커피 재화 아이콘
    icon_coffee_1 = al_create_sub_bitmap(spr_items, C1X, C1Y, CWH, CWH);
    icon_coffee_2 = al_create_sub_bitmap(spr_items, C2X, C2Y, CWH, CWH);
    icon_coffee_3 = al_create_sub_bitmap(spr_items, C3X, C3Y, CWH, CWH);
    //졸고 있는 학생 아이콘
    icon_sleeping = al_create_sub_bitmap(spr_items, P0X, P0Y, PWH, PWH);
    //아군 유닛(타워) 아이콘
    icon_people1 = al_create_sub_bitmap(spr_items, P0X, P1Y, PWH, PWH); // 원거리
    icon_people2 = al_create_sub_bitmap(spr_items, P0X, P2Y, PWH, PWH); // 보조
    icon_people3 = al_create_sub_bitmap(spr_items, P0X, P3Y, PWH, PWH); // 탱커
    icon_people1_1 = al_create_sub_bitmap(spr_items, P1X, P1Y, PWH, PWH);
    icon_people2_1 = al_create_sub_bitmap(spr_items, P1X, P2Y, PWH, PWH);
    icon_people3_1 = al_create_sub_bitmap(spr_items, P1X, P3Y, PWH, PWH);
    //바이러스 아이콘
    icon_virus1 = al_create_sub_bitmap(spr_items, V1X, V1Y, PWH, PWH); // 파
    icon_virus2 = al_create_sub_bitmap(spr_items, V1X, V2Y, PWH, PWH); // 노
    icon_virus3 = al_create_sub_bitmap(spr_items, V1X, V3Y, PWH, PWH); // 빨
    icon_virus4 = al_create_sub_bitmap(spr_items, V1X, V4Y, PWH, PWH); // 하
    //커피콩(재화)아이콘
    icon_coffee_bean = al_create_sub_bitmap(spr_items, BEAN_X, BEAN_Y, BEAN_W, BEAN_H);
    //총알 아이콘
    bullet_1 = al_create_sub_bitmap(spr_items, B1X, BY, BW, BH);
    bullet_2 = al_create_sub_bitmap(spr_items, B2X, BY, BW, BH);
    bullet_3 = al_create_sub_bitmap(spr_items, B3X, BY, BW, BH);
    //라이프게이지 아이콘
    icon_lifegauge1 = al_create_sub_bitmap(spr_items, L1X, L1Y, LW, LH); // 5칸
    icon_lifegauge2 = al_create_sub_bitmap(spr_items, L2X, L1Y, LW, LH); // 4칸
    icon_lifegauge3 = al_create_sub_bitmap(spr_items, L1X, L2Y, LW, LH); // 3칸
    icon_lifegauge4 = al_create_sub_bitmap(spr_items, L2X, L2Y, LW, LH); // 2칸
    icon_lifegauge5 = al_create_sub_bitmap(spr_items, L1X, L3Y, LW, LH); // 1칸
    icon_lifegauge6 = al_create_sub_bitmap(spr_items, L2X, L3Y, LW, LH); // 0칸
    //자폭 바이러스 폭발 이펙트 아이콘
    icon_bombeffect1 = al_create_sub_bitmap(spr_items, BOMBX1, BOMBY1, BWH1, BWH1); // 폭탄 바이러스 터질 때 이펙트
    icon_bombeffect2 = al_create_sub_bitmap(spr_items, BOMBX2, BOMBY2, BWH2, BWH2);
    icon_bombeffect3 = al_create_sub_bitmap(spr_items, BOMBX3, BOMBY3, BWH3, BWH3);
    //냉동 바이러스 얼림 이펙트 아이콘
    icon_frozen1 = al_create_sub_bitmap(spr_items, FPX1, FPY1, FPWH, FPWH);
    icon_frozen2 = al_create_sub_bitmap(spr_items, FPX1, FPY2, FPWH, FPWH);
    icon_frozen3 = al_create_sub_bitmap(spr_items, FPX2, FPY2, FPWH, FPWH);
    if (!bullet_1 || !bullet_2 || !bullet_3) return false; // 총알 이미지 체크

    return true;
}

// 게임 종료 시 이미지 해제
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
    if (icon_people1_1) al_destroy_bitmap(icon_people1_1);
    if (icon_people2_1) al_destroy_bitmap(icon_people2_1);
    if (icon_people3_1) al_destroy_bitmap(icon_people3_1);
    if (icon_lifegauge1) al_destroy_bitmap(icon_lifegauge1);
    if (icon_lifegauge2) al_destroy_bitmap(icon_lifegauge2);
    if (icon_lifegauge3) al_destroy_bitmap(icon_lifegauge3);
    if (icon_lifegauge4) al_destroy_bitmap(icon_lifegauge4);
    if (icon_lifegauge5) al_destroy_bitmap(icon_lifegauge5);
    if (icon_lifegauge6) al_destroy_bitmap(icon_lifegauge6);
    if (icon_bombeffect1) al_destroy_bitmap(icon_bombeffect1);
    if (icon_bombeffect2) al_destroy_bitmap(icon_bombeffect2);
    if (icon_bombeffect3) al_destroy_bitmap(icon_bombeffect3);
    if (icon_frozen1) al_destroy_bitmap(icon_frozen1);
    if (icon_frozen2) al_destroy_bitmap(icon_frozen2);
    if (icon_frozen3) al_destroy_bitmap(icon_frozen3);
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
    icon_people1_1 = icon_people2_1 = icon_people3_1 = NULL;
    icon_lifegauge1 = icon_lifegauge2 = icon_lifegauge3 = NULL;
    icon_lifegauge4 = icon_lifegauge5 = icon_lifegauge6 = NULL;
    icon_bombeffect1 = icon_bombeffect2 = icon_bombeffect3 = NULL;
    icon_frozen1 = icon_frozen2 = icon_frozen3 = NULL;
    font_title = font_ui = NULL;
}
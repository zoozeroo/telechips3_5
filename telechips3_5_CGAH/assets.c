#include "assets.h"
#include <allegro5/allegro_image.h>

ALLEGRO_BITMAP* bg_home = NULL;
ALLEGRO_BITMAP* bg_rank = NULL;
ALLEGRO_BITMAP* bg_play = NULL;

ALLEGRO_FONT* font_title = NULL;
ALLEGRO_FONT* font_ui = NULL;

bool assets_load(void) {
    font_title = al_create_builtin_font();                      //제목 : 기본 폰트 사용
    font_ui = al_create_builtin_font();                         //UI : 기본 폰트 사용
    if (!font_title || !font_ui) return false;

    bg_home = al_load_bitmap("background_play.png");                    //메인화면 배경 이미지 -> 일단 게임 플레이 화면으로 통일해둠
    bg_rank = al_load_bitmap("background_play.png");                        //랭킹화면 배경 이미지
    bg_play = al_load_bitmap("background_play.png");                        //게임 플레이 배경 이미지
    return true;
}

void assets_unload(void) {                                      //게임 창 해제
    if (bg_home) al_destroy_bitmap(bg_home);
    if (bg_rank) al_destroy_bitmap(bg_rank);
    if (bg_play) al_destroy_bitmap(bg_play);
    if (font_title) al_destroy_font(font_title);
    if (font_ui)    al_destroy_font(font_ui);

    bg_home = bg_rank = bg_play = NULL;
    font_title = font_ui = NULL;
}

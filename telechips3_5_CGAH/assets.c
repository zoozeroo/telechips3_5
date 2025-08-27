#include "assets.h"
#include <allegro5/allegro_image.h>

ALLEGRO_BITMAP* bg_home = NULL;
ALLEGRO_BITMAP* bg_rank = NULL;
ALLEGRO_BITMAP* bg_play = NULL;

ALLEGRO_FONT* font_title = NULL;
ALLEGRO_FONT* font_ui = NULL;

bool assets_load(void) {
    font_title = al_create_builtin_font();                      //���� : �⺻ ��Ʈ ���
    font_ui = al_create_builtin_font();                         //UI : �⺻ ��Ʈ ���
    if (!font_title || !font_ui) return false;

    bg_home = al_load_bitmap("background_play.png");                    //����ȭ�� ��� �̹��� -> �ϴ� ���� �÷��� ȭ������ �����ص�
    bg_rank = al_load_bitmap("background_play.png");                        //��ŷȭ�� ��� �̹���
    bg_play = al_load_bitmap("background_play.png");                        //���� �÷��� ��� �̹���
    return true;
}

void assets_unload(void) {                                      //���� â ����
    if (bg_home) al_destroy_bitmap(bg_home);
    if (bg_rank) al_destroy_bitmap(bg_rank);
    if (bg_play) al_destroy_bitmap(bg_play);
    if (font_title) al_destroy_font(font_title);
    if (font_ui)    al_destroy_font(font_ui);

    bg_home = bg_rank = bg_play = NULL;
    font_title = font_ui = NULL;
}

#pragma once
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

extern ALLEGRO_BITMAP* bg_home;
extern ALLEGRO_BITMAP* bg_rank;
extern ALLEGRO_BITMAP* bg_play;

extern ALLEGRO_FONT* font_title;
extern ALLEGRO_FONT* font_ui;

bool assets_load(void);   // ��Ʈ/��� �ε�
void assets_unload(void); // ����

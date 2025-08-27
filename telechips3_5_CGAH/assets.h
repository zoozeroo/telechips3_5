#pragma once
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

extern ALLEGRO_BITMAP* bg_home;
extern ALLEGRO_BITMAP* bg_rank;
extern ALLEGRO_BITMAP* bg_play;

extern ALLEGRO_FONT* font_title;
extern ALLEGRO_FONT* font_ui;

bool assets_load(void);   // 폰트/배경 로드
void assets_unload(void); // 해제

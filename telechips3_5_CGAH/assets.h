#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

extern ALLEGRO_BITMAP* bg_home;
extern ALLEGRO_BITMAP* bg_rank;
extern ALLEGRO_BITMAP* bg_play;
extern ALLEGRO_FONT* font_title;
extern ALLEGRO_FONT* font_ui;
extern ALLEGRO_BITMAP* spr_items;
extern ALLEGRO_BITMAP* icon_coffee_1;
extern ALLEGRO_BITMAP* icon_coffee_2;
extern ALLEGRO_BITMAP* icon_coffee_3;
extern ALLEGRO_BITMAP* icon_sleeping;
extern ALLEGRO_BITMAP* icon_people1;
extern ALLEGRO_BITMAP* icon_people2;
extern ALLEGRO_BITMAP* icon_people3;
extern ALLEGRO_BITMAP* icon_people1_1;
extern ALLEGRO_BITMAP* icon_people2_1;
extern ALLEGRO_BITMAP* icon_people3_1;
extern ALLEGRO_BITMAP* icon_virus1;
extern ALLEGRO_BITMAP* icon_virus2;
extern ALLEGRO_BITMAP* icon_virus3;
extern ALLEGRO_BITMAP* icon_virus4;
extern ALLEGRO_BITMAP* icon_lifegauge1;
extern ALLEGRO_BITMAP* icon_lifegauge2;
extern ALLEGRO_BITMAP* icon_lifegauge3;
extern ALLEGRO_BITMAP* icon_lifegauge4;
extern ALLEGRO_BITMAP* icon_lifegauge5;
extern ALLEGRO_BITMAP* icon_lifegauge6;
extern ALLEGRO_BITMAP* icon_coffee_bean;

bool assets_load(void);
void assets_unload(void);
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
extern ALLEGRO_BITMAP* icon_coffee_bean;

bool assets_load(void);
void assets_unload(void);
#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

extern ALLEGRO_BITMAP* bg_home;
extern ALLEGRO_BITMAP* bg_rank;
extern ALLEGRO_BITMAP* bg_play;
extern ALLEGRO_FONT* font_title;
extern ALLEGRO_FONT* font_ui;
extern ALLEGRO_BITMAP* spr_items;

// Ŀ�� ������
extern ALLEGRO_BITMAP* icon_coffee_1;
extern ALLEGRO_BITMAP* icon_coffee_2;
extern ALLEGRO_BITMAP* icon_coffee_3;

// �Ʊ� ���� (�ڴ� ���)
extern ALLEGRO_BITMAP* icon_sleeping;

// �Ʊ� ����
extern ALLEGRO_BITMAP* icon_people1;
extern ALLEGRO_BITMAP* icon_people2;
extern ALLEGRO_BITMAP* icon_people3;

// �� ����
extern ALLEGRO_BITMAP* icon_virus1;
extern ALLEGRO_BITMAP* icon_virus2;
extern ALLEGRO_BITMAP* icon_virus3;
extern ALLEGRO_BITMAP* icon_virus4;

// Ŀ����
extern ALLEGRO_BITMAP* icon_coffee_bean;

// �Ѿ� ����
extern ALLEGRO_BITMAP* bullet_1;
extern ALLEGRO_BITMAP* bullet_2;
extern ALLEGRO_BITMAP* bullet_3;

// ���� ������ ����
extern ALLEGRO_BITMAP* icon_lifegauge1;
extern ALLEGRO_BITMAP* icon_lifegauge2;
extern ALLEGRO_BITMAP* icon_lifegauge3;
extern ALLEGRO_BITMAP* icon_lifegauge4;
extern ALLEGRO_BITMAP* icon_lifegauge5;
extern ALLEGRO_BITMAP* icon_lifegauge6;

// ���� �ִϸ��̼ǿ� �߰� �̹���
extern ALLEGRO_BITMAP* icon_people1_1;
extern ALLEGRO_BITMAP* icon_people2_1;
extern ALLEGRO_BITMAP* icon_people3_1;

bool assets_load(void);
void assets_unload(void);
#pragma once
#define VIRUS_W  34
#define VIRUS_H  34

#define BEAN_W   25
#define BEAN_H   25

#define TEXT_W   9
#define TEXT_H   9

#define ITEM_W  25
#define ITEM_H  25

// 기본 (자는상태)
#define STUDENT_W 34
#define STUDENT_H 34

// 유닛들(변신상태)
#define UNIT_W   34
#define UNIT_H   34

#define HPBAR_W   40
#define HPBAR_H   10
#define HPBAR_FRAMES 6

#define VIRUS_FRAMES 2
#define UNIT_FRAMES  2
#define TEXT_FRAMES  3

typedef struct SPRITES
{
    ALLEGRO_BITMAP* _sheet;

    // 적군
    ALLEGRO_BITMAP* virus_blue[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_yellow[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_red[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_cyan[VIRUS_FRAMES];

    // 아군 유닛
    ALLEGRO_BITMAP* americano;            // 원거리 1프레임
    ALLEGRO_BITMAP* caffe_latte;          // 탱커 1프레임
    ALLEGRO_BITMAP* caramel_macchiato;    // 보조 1프레임

    // 자원
    ALLEGRO_BITMAP* coffee_bean;

    // 공격 텍스트
    ALLEGRO_BITMAP* answer_text[TEXT_FRAMES];

    // 학생
    ALLEGRO_BITMAP* student_sleeping;

    // 변신 유닛 애니메이션
    ALLEGRO_BITMAP* unit_ranged[UNIT_FRAMES];
    ALLEGRO_BITMAP* unit_tank[UNIT_FRAMES];
    ALLEGRO_BITMAP* unit_support[UNIT_FRAMES];

    // 체력바
    ALLEGRO_BITMAP* hpbar[HPBAR_FRAMES];

} SPRITES;

SPRITES sprites;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h);

void sprites_init();
void sprites_deinit();

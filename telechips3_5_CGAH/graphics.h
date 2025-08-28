#pragma once
#define VIRUS_W  34
#define VIRUS_H  34

#define BEAN_W   25
#define BEAN_H   25

#define TEXT_W   9
#define TEXT_H   9

#define ITEM_W  25
#define ITEM_H  25

// �⺻ (�ڴ»���)
#define STUDENT_W 34
#define STUDENT_H 34

// ���ֵ�(���Ż���)
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

    // ����
    ALLEGRO_BITMAP* virus_blue[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_yellow[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_red[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_cyan[VIRUS_FRAMES];

    // �Ʊ� ����
    ALLEGRO_BITMAP* americano;            // ���Ÿ� 1������
    ALLEGRO_BITMAP* caffe_latte;          // ��Ŀ 1������
    ALLEGRO_BITMAP* caramel_macchiato;    // ���� 1������

    // �ڿ�
    ALLEGRO_BITMAP* coffee_bean;

    // ���� �ؽ�Ʈ
    ALLEGRO_BITMAP* answer_text[TEXT_FRAMES];

    // �л�
    ALLEGRO_BITMAP* student_sleeping;

    // ���� ���� �ִϸ��̼�
    ALLEGRO_BITMAP* unit_ranged[UNIT_FRAMES];
    ALLEGRO_BITMAP* unit_tank[UNIT_FRAMES];
    ALLEGRO_BITMAP* unit_support[UNIT_FRAMES];

    // ü�¹�
    ALLEGRO_BITMAP* hpbar[HPBAR_FRAMES];

} SPRITES;

SPRITES sprites;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h);

void sprites_init();
void sprites_deinit();

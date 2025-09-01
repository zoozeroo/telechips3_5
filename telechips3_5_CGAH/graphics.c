#include <allegro5/allegro5.h>
#include "graphics.h"


// --- sprites for Sleep Defense ---

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void must_init(bool test, const char* description)
{
    if (!test)
    {
        printf("couldn't initialize %s\n", description);
        exit(1);
    }
}


ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h)
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
    must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init()
{
    sprites._sheet = al_load_bitmap("spritesheet.png");
    must_init(sprites._sheet, "spritesheet");

    // --- ���� ---
    sprites.virus_blue[0] = sprite_grab(8, 3, VIRUS_W, VIRUS_H);
    sprites.virus_blue[1] = sprite_grab(16, 0, VIRUS_W, VIRUS_H);

    sprites.virus_yellow[0] = sprite_grab(32, 0, VIRUS_W, VIRUS_H);
    sprites.virus_yellow[1] = sprite_grab(48, 0, VIRUS_W, VIRUS_H);

    sprites.virus_red[0] = sprite_grab(0, 16, VIRUS_W, VIRUS_H);
    sprites.virus_red[1] = sprite_grab(16, 16, VIRUS_W, VIRUS_H);

    sprites.virus_cyan[0] = sprite_grab(32, 16, VIRUS_W, VIRUS_H);
    sprites.virus_cyan[1] = sprite_grab(48, 16, VIRUS_W, VIRUS_H);

    // --- �Ʊ� ���� �⺻ ������ ---
    sprites.americano = sprite_grab(0, 32, ITEM_W, ITEM_H);
    sprites.caffe_latte = sprite_grab(24, 32, ITEM_W, ITEM_H);
    sprites.caramel_macchiato = sprite_grab(48, 32, ITEM_W, ITEM_H);

    // --- �ڿ� ---
    sprites.coffee_bean = sprite_grab(0, 56, BEAN_W, BEAN_H);

    // --- ���� �ؽ�Ʈ (���� �߻�) ---
    sprites.answer_text[0] = sprite_grab(0, 72, TEXT_W, TEXT_H);
    sprites.answer_text[1] = sprite_grab(16, 72, TEXT_W, TEXT_H);
    sprites.answer_text[2] = sprite_grab(32, 72, TEXT_W, TEXT_H);

    // --- �л� ---
    sprites.student_sleeping = sprite_grab(91, 3, STUDENT_W, STUDENT_H); // 91, 3

    // --- ���� ���� (�ִϸ��̼� 2�����Ӿ�) ---
    sprites.unit_ranged[0] = sprite_grab(91, 39, UNIT_W, UNIT_H); // 91, 39
    sprites.unit_ranged[1] = sprite_grab(127, 39, UNIT_W, UNIT_H); // 127

    sprites.unit_tank[0] = sprite_grab(91, 76, UNIT_W, UNIT_H); 
    sprites.unit_tank[1] = sprite_grab(127, 76, UNIT_W, UNIT_H);

    sprites.unit_support[0] = sprite_grab(91, 111, UNIT_W, UNIT_H);
    sprites.unit_support[1] = sprite_grab(127, 111, UNIT_W, UNIT_H);

    // --- ü�¹� (6������) ---
    for (int i = 0; i < HPBAR_FRAMES; i++) {
        sprites.hpbar[i] = sprite_grab(0 + i * HPBAR_W, 120, HPBAR_W, HPBAR_H);
    }
}

void sprites_deinit()
{
    // ����
    for (int i = 0; i < VIRUS_FRAMES; i++) {
        al_destroy_bitmap(sprites.virus_blue[i]);
        al_destroy_bitmap(sprites.virus_yellow[i]);
        al_destroy_bitmap(sprites.virus_red[i]);
        al_destroy_bitmap(sprites.virus_cyan[i]);
    }

    // �Ʊ�
    al_destroy_bitmap(sprites.americano);
    al_destroy_bitmap(sprites.caffe_latte);
    al_destroy_bitmap(sprites.caramel_macchiato);

    // �ڿ�
    al_destroy_bitmap(sprites.coffee_bean);

    // �ؽ�Ʈ
    for (int i = 0; i < TEXT_FRAMES; i++) {
        al_destroy_bitmap(sprites.answer_text[i]);
    }

    // �л�
    al_destroy_bitmap(sprites.student_sleeping);

    // ���� ����
    for (int i = 0; i < UNIT_FRAMES; i++) {
        al_destroy_bitmap(sprites.unit_ranged[i]);
        al_destroy_bitmap(sprites.unit_tank[i]);
        al_destroy_bitmap(sprites.unit_support[i]);
    }

    // ü�¹�
    for (int i = 0; i < HPBAR_FRAMES; i++) {
        al_destroy_bitmap(sprites.hpbar[i]);
    }

    // ���� ��Ʈ
    al_destroy_bitmap(sprites._sheet);
}
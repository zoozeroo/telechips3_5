#pragma once
#include <stdbool.h>

#define NAME_MAX   32
#define MAX_SCORE  100
// SCORE_FILE �� app.h � �� ���� �����ϼ���.
// #define SCORE_FILE "score.csv"  // <- �ߺ��̸� �ּ� ó��/����

// ��ŷ �׸�: stage(�������� �켱) -> time(�������� ����)
typedef struct {
    char name[NAME_MAX];
    int  stage;   // Ŭ������ �������� ��
    int  time;    // �÷��� �ð�(��)
} Entry;

void score_load(const char* path);
void score_add_and_save(int seconds, const char* name, const char* path);
int   score_count_get(void);
Entry score_get(int index);

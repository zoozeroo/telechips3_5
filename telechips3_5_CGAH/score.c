#define _CRT_SECURE_NO_WARNINGS
#include "score.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Entry s_highscores[MAX_SCORE];           //��ŷ ȭ�鿡 ��µ� ���̽��ھ� ���
static int   s_count = 0;                                     //���ھ� ���� : 10�� ����

static int cmp_asc(const void* a, const void* b) {             //qsort�� �Լ� : a, b ���ؼ� �� ū �� ���� -> ������ �÷���Ÿ������ �ٲ� ��. ���߿� �����ϱ� -> ��������!
    const Entry* A = (const Entry*)a;
    const Entry* B = (const Entry*)b;
    return A->score - B->score;
}

void score_load(const char* path) {                             //���� �ҷ�����
    s_count = 0;
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    char line[256];                                                     //�̸�, ���� ��Ͽ�
    while (s_count < MAX_SCORE && fgets(line, sizeof line, f)) {
        Entry e; e.name[0] = '\0'; e.score = 0;
        if (sscanf_s(line, " %31[^,],%d", e.name, (unsigned)NAME_MAX, &e.score) == 2) {
            s_highscores[s_count++] = e;
        }
    }
    fclose(f);
    qsort(s_highscores, s_count, sizeof(Entry), cmp_asc);          //��ŷ ���������� ����
}

void score_add_and_save(int s, const char* name, const char* path) {            //strcpy�� �÷��̾� �г��� �ް� �迭 �������� null �߰�
    Entry e; e.score = s;
    if (!name || !name[0]) strncpy(e.name, "PLAYER", NAME_MAX);
    else                   strncpy(e.name, name, NAME_MAX);
    e.name[NAME_MAX - 1] = '\0';

    if (s_count < MAX_SCORE) s_highscores[s_count++] = e;           //��ŷ 10���� ����
    else                     s_highscores[s_count - 1] = e;

    qsort(s_highscores, s_count, sizeof(Entry), cmp_asc);              //��ŷ ���������� ����

    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;                              //scores.txt ���Ͽ� ����, �г��� ����
    for (int i = 0; i < s_count && i < MAX_SCORE; ++i) {
        fprintf(f, "%s,%d\n", s_highscores[i].name, s_highscores[i].score);
    }
    fclose(f);
}

int score_count_get(void) { return s_count; }                           // �ִ� 10�� ����

Entry score_get(int index) {                                                    //��
    Entry e; e.score = 0; e.name[0] = '\0';
    if (index >= 0 && index < s_count) return s_highscores[index];
    return e;
}

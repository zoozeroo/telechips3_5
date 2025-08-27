#define _CRT_SECURE_NO_WARNINGS
#include "score.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Entry s_highscores[MAX_SCORE];           //랭킹 화면에 출력될 하이스코어 목록
static int   s_count = 0;                                     //스코어 갯수 : 10개 제한

static int cmp_asc(const void* a, const void* b) {             //qsort용 함수 : a, b 비교해서 더 큰 값 리턴 -> 점수를 플레이타임으로 바꿀 것. 나중에 변경하기 -> 변경했음!
    const Entry* A = (const Entry*)a;
    const Entry* B = (const Entry*)b;
    return A->score - B->score;
}

void score_load(const char* path) {                             //점수 불러오기
    s_count = 0;
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    char line[256];                                                     //이름, 점수 기록용
    while (s_count < MAX_SCORE && fgets(line, sizeof line, f)) {
        Entry e; e.name[0] = '\0'; e.score = 0;
        if (sscanf_s(line, " %31[^,],%d", e.name, (unsigned)NAME_MAX, &e.score) == 2) {
            s_highscores[s_count++] = e;
        }
    }
    fclose(f);
    qsort(s_highscores, s_count, sizeof(Entry), cmp_asc);          //랭킹 점수순으로 정렬
}

void score_add_and_save(int s, const char* name, const char* path) {            //strcpy로 플레이어 닉네임 받고 배열 마지막에 null 추가
    Entry e; e.score = s;
    if (!name || !name[0]) strncpy(e.name, "PLAYER", NAME_MAX);
    else                   strncpy(e.name, name, NAME_MAX);
    e.name[NAME_MAX - 1] = '\0';

    if (s_count < MAX_SCORE) s_highscores[s_count++] = e;           //랭킹 10개로 제한
    else                     s_highscores[s_count - 1] = e;

    qsort(s_highscores, s_count, sizeof(Entry), cmp_asc);              //랭킹 점수순으로 정렬

    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;                              //scores.txt 파일에 점수, 닉네임 저장
    for (int i = 0; i < s_count && i < MAX_SCORE; ++i) {
        fprintf(f, "%s,%d\n", s_highscores[i].name, s_highscores[i].score);
    }
    fclose(f);
}

int score_count_get(void) { return s_count; }                           // 최대 10개 리턴

Entry score_get(int index) {                                                    //흠
    Entry e; e.score = 0; e.name[0] = '\0';
    if (index >= 0 && index < s_count) return s_highscores[index];
    return e;
}

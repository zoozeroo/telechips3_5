#define _CRT_SECURE_NO_WARNINGS
#include "score.h"
#include "game.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Entry s_highscores[MAX_SCORE];
static int   s_count = 0;

// 랭킹 비교하는 함수. qsort용 내림차순 정렬
static int cmp_rank(const void* a, const void* b) {
    const Entry* A = (const Entry*)a;
    const Entry* B = (const Entry*)b;
    if (A->stage != B->stage) return B->stage - A->stage; 
    return A->time - B->time;                              
}

//txt 파일 열어서 점수 불러오는 함수
void score_load(const char* path) {
    s_count = 0;
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    char line[256];
    while (s_count < MAX_SCORE && fgets(line, sizeof line, f)) {
        Entry e; e.name[0] = '\0'; e.stage = 0; e.time = 0;

        int n = sscanf_s(line, " %31[^,],%d,%d", e.name, (unsigned)NAME_MAX, &e.stage, &e.time);
        if (n == 3) {
            s_highscores[s_count++] = e;
            continue;
        }

        int only_time = 0;
        if (sscanf_s(line, " %31[^,],%d", e.name, (unsigned)NAME_MAX, &only_time) == 2) {
            e.stage = 0;
            e.time = only_time;
            s_highscores[s_count++] = e;
        }
    }
    fclose(f);
    qsort(s_highscores, s_count, sizeof(Entry), cmp_rank);      //qsort로 랭킹 상위 10개 정렬
}

//점수 더하고 저장하는 함수
void score_add_and_save(int seconds, const char* name, const char* path) {
    GameState gs = game_get_state();
    int cleared = gs.stage;
    if (!gs.cleared) cleared = gs.stage - 1;
    if (cleared < 0) cleared = 0;

    Entry e;
    e.time = seconds;
    e.stage = cleared;

    if (!name || !name[0]) strncpy(e.name, "PLAYER", NAME_MAX);         //플레이어가 이름 입력을 하지 않았을 때 : 그냥 PLAYER로 표시함
    else                   strncpy(e.name, name, NAME_MAX);
    e.name[NAME_MAX - 1] = '\0';                                        //마지막 null 처리

    if (s_count < MAX_SCORE) s_highscores[s_count++] = e;
    else                     s_highscores[s_count - 1] = e;

    qsort(s_highscores, s_count, sizeof(Entry), cmp_rank);

    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;
    for (int i = 0; i < s_count && i < MAX_SCORE; ++i) {
        fprintf(f, "%s,%d,%d\n", s_highscores[i].name, s_highscores[i].stage, s_highscores[i].time);
    }
    fclose(f);
}

int score_count_get(void) { return s_count; }                   //점수 갯수 세기

Entry score_get(int index) {                                
    Entry e; e.time = 0; e.stage = 0; e.name[0] = '\0';
    if (index >= 0 && index < s_count) return s_highscores[index];
    return e;
}

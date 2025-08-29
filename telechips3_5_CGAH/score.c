#define _CRT_SECURE_NO_WARNINGS
#include "score.h"
#include "game.h"  // game_get_state()로 현재 스테이지 읽기
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Entry s_highscores[MAX_SCORE];
static int   s_count = 0;

// 정렬: stage 내림차순 → time 오름차순
static int cmp_rank(const void* a, const void* b) {
    const Entry* A = (const Entry*)a;
    const Entry* B = (const Entry*)b;
    if (A->stage != B->stage) return B->stage - A->stage; // stage 큰게 먼저
    return A->time - B->time;                              // time 작은게 먼저
}

void score_load(const char* path) {
    s_count = 0;
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    char line[256];
    while (s_count < MAX_SCORE && fgets(line, sizeof line, f)) {
        Entry e; e.name[0] = '\0'; e.stage = 0; e.time = 0;

        // 새 포맷: name,stage,time
        int n = sscanf_s(line, " %31[^,],%d,%d", e.name, (unsigned)NAME_MAX, &e.stage, &e.time);
        if (n == 3) {
            s_highscores[s_count++] = e;
            continue;
        }

        // 구 포맷 하위호환: name,time  → stage=0 간주
        int only_time = 0;
        if (sscanf_s(line, " %31[^,],%d", e.name, (unsigned)NAME_MAX, &only_time) == 2) {
            e.stage = 0;
            e.time = only_time;
            s_highscores[s_count++] = e;
        }
    }
    fclose(f);
    qsort(s_highscores, s_count, sizeof(Entry), cmp_rank);
}

void score_add_and_save(int seconds, const char* name, const char* path) {
    // 현재 게임 상태로부터 '클리어한 스테이지 수' 계산
    GameState gs = game_get_state();
    int cleared = gs.stage;
    if (!gs.cleared) cleared = gs.stage - 1;   // 실패 시 진행 중 스테이지는 미클리어
    if (cleared < 0) cleared = 0;

    Entry e;
    e.time = seconds;
    e.stage = cleared;

    if (!name || !name[0]) strncpy(e.name, "PLAYER", NAME_MAX);
    else                   strncpy(e.name, name, NAME_MAX);
    e.name[NAME_MAX - 1] = '\0';

    if (s_count < MAX_SCORE) s_highscores[s_count++] = e;
    else                     s_highscores[s_count - 1] = e;

    qsort(s_highscores, s_count, sizeof(Entry), cmp_rank);

    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;
    for (int i = 0; i < s_count && i < MAX_SCORE; ++i) {
        // 항상 새 포맷으로 저장
        fprintf(f, "%s,%d,%d\n", s_highscores[i].name, s_highscores[i].stage, s_highscores[i].time);
    }
    fclose(f);
}

int score_count_get(void) { return s_count; }

Entry score_get(int index) {
    Entry e; e.time = 0; e.stage = 0; e.name[0] = '\0';
    if (index >= 0 && index < s_count) return s_highscores[index];
    return e;
}

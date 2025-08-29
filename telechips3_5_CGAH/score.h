#pragma once
#include <stdbool.h>

#define NAME_MAX   32
#define MAX_SCORE  100
// SCORE_FILE 은 app.h 등에 한 번만 정의하세요.
// #define SCORE_FILE "score.csv"  // <- 중복이면 주석 처리/삭제

// 랭킹 항목: stage(내림차순 우선) -> time(오름차순 보조)
typedef struct {
    char name[NAME_MAX];
    int  stage;   // 클리어한 스테이지 수
    int  time;    // 플레이 시간(초)
} Entry;

void score_load(const char* path);
void score_add_and_save(int seconds, const char* name, const char* path);
int   score_count_get(void);
Entry score_get(int index);

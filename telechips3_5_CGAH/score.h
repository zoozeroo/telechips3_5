#pragma once
#include <stdbool.h>

#define MAX_SCORE 1000
#define NAME_MAX  32
#define SCORE_FILE "scores.txt"

typedef struct {
    int  score;
    char name[NAME_MAX];
} Entry;

void score_load(const char* path);
void score_add_and_save(int s, const char* name, const char* path);

int   score_count_get(void);
Entry score_get(int index);

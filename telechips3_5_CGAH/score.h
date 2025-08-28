#pragma once
#define MAX_SCORE 10
#define NAME_MAX 32

typedef struct { char name[NAME_MAX]; int score; } Entry;

void score_load(const char* path);
void score_add_and_save(int s, const char* name, const char* path);
int score_count_get(void);
Entry score_get(int index);

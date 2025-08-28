#pragma once
#include "app.h"  // Rect 타입을 위해 app.h 포함

void draw_menu(int W, int H, Rect btn_start, Rect btn_howto, Rect btn_rank, float mx, float my);
void draw_play(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, const int marks[GRID_ROWS][GRID_COLS]);
void draw_howto(int W, int H);
void draw_rank(int W, int H);
void draw_end(int W, int H, const char* name_buf, int score_second, bool success);
void draw_play_with_game(int W, int H, int score_second, int sel_col, int sel_row, int selected_item, bool show_ranges);
void draw_pause_overlay(int W, int H, Rect btn_resume, Rect btn_main, int selected, float mx, float my);

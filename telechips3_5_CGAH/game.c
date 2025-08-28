#include "game.h"
#include "assets.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

// 그리드 레이아웃
#define CELL_W 80
#define CELL_H 67
#define GRID_X 280
#define GRID_Y 240

static Tower grid[GRID_ROWS][GRID_COLS];
static Enemy enemies[MAX_ENEMIES];
static GameState game_state;
static double last_enemy_spawn_time = 0.0;

static inline int tower_max_hp(TowerType t) {
    if (t == TOWER_ATTACK) return ATTACK_TOWER_HP;
    if (t == TOWER_RESOURCE) return RESOURCE_TOWER_HP;
    if (t == TOWER_TANK) return TANK_TOWER_HP;
    return 0;
}

static void cell_rect(int row, int col, float* x1, float* y1, float* x2, float* y2) {
    *x1 = GRID_X + col * CELL_W;
    *y1 = GRID_Y + row * CELL_H;
    *x2 = *x1 + CELL_W;
    *y2 = *y1 + CELL_H;
}

static float distance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void game_init(void) {
    srand(time(NULL));

    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c].type = TOWER_EMPTY;
            grid[r][c].cooldown = 0.0f;
            grid[r][c].hp = 0;
        }
    }
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
        enemies[i].atk_cooldown = 0.0f;
    }
    game_state.caffeine = 200;
    game_state.lives = 10;
    game_state.stage = 1;
    game_state.stage_kills = 0;
    game_state.cleared = false;
    game_state.game_over = false;

    last_enemy_spawn_time = al_get_time();
}

void game_reset(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
    }
    game_state.game_over = false;
    game_state.cleared = false;
}

static void spawn_enemy() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].x = GRID_X + GRID_COLS * CELL_W + 20; // 오른쪽에서 시작
            enemies[i].y = GRID_Y + (rand() % (int)(GRID_ROWS * CELL_H));
            enemies[i].hp = ENEMY_HP;
            enemies[i].atk_cooldown = 0.0f;
            break;
        }
    }
}

static void on_enemy_killed(void) {
    game_state.stage_kills++;
    if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
        // 모든 적 제거
        for (int i = 0; i < MAX_ENEMIES; ++i) {
            enemies[i].active = false;
        }

        if (game_state.stage >= MAX_STAGES) {
            game_state.cleared = true;
            return;
        }

        game_state.stage++;
        game_state.stage_kills = 0;
        last_enemy_spawn_time = al_get_time();
    }
}

void game_update(float dt) {
    if (game_state.cleared || game_state.game_over) return;

    // 타워 로직
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float tower_cx = GRID_X + c * CELL_W + CELL_W / 2.0f;
            float tower_cy = GRID_Y + r * CELL_H + CELL_H / 2.0f;

            if (grid[r][c].cooldown > 0) {
                grid[r][c].cooldown -= dt;
            }

            if (grid[r][c].type == TOWER_ATTACK && grid[r][c].cooldown <= 0) {
                Enemy* target = NULL;
                float min_dist = ATTACK_TOWER_RANGE;

                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (enemies[i].active) {
                        float d = distance(tower_cx, tower_cy, enemies[i].x, enemies[i].y);
                        if (d < min_dist) {
                            min_dist = d;
                            target = &enemies[i];
                        }
                    }
                }

                if (target) {
                    target->hp -= ATTACK_TOWER_DAMAGE;
                    if (target->hp <= 0) {
                        target->active = false;
                        game_state.caffeine += 20;
                        on_enemy_killed();
                    }
                    grid[r][c].cooldown = ATTACK_TOWER_COOLDOWN;
                }
            }
            else if (grid[r][c].type == TOWER_RESOURCE && grid[r][c].cooldown <= 0) {
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                grid[r][c].cooldown = RESOURCE_TOWER_COOLDOWN;
            }
        }
    }

    // 적 로직
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (enemies[i].atk_cooldown > 0.0f) {
            enemies[i].atk_cooldown -= dt;
        }

        Tower* target_cell = NULL;
        float best_d = ENEMY_ATTACK_RANGE;

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_EMPTY) continue;
                float x1, y1, x2, y2;
                cell_rect(r, c, &x1, &y1, &x2, &y2);
                float tcx = (x1 + x2) * 0.5f;
                float tcy = (y1 + y2) * 0.5f;
                float d = distance(enemies[i].x, enemies[i].y, tcx, tcy);
                if (d < best_d) {
                    best_d = d;
                    target_cell = &grid[r][c];
                }
            }
        }

        if (target_cell != NULL) {
            // 공격
            if (enemies[i].atk_cooldown <= 0.0f) {
                target_cell->hp -= ENEMY_ATTACK_DAMAGE;
                enemies[i].atk_cooldown = ENEMY_ATTACK_COOLDOWN;

                if (target_cell->hp <= 0) {
                    target_cell->type = TOWER_EMPTY;
                    target_cell->cooldown = 0.0f;
                    target_cell->hp = 0;
                }
            }
        }
        else {
            // 이동
            enemies[i].x -= ENEMY_SPEED;
            if (enemies[i].x <= GRID_X - 20) {
                enemies[i].active = false;
                game_state.lives--;
                if (game_state.lives <= 0) {
                    game_state.game_over = true;
                }
            }
        }
    }

    // 적 스폰
    if (game_state.stage_kills < KILLS_TO_ADVANCE &&
        (al_get_time() - last_enemy_spawn_time > 2.0)) {
        spawn_enemy();
        last_enemy_spawn_time = al_get_time();
    }
}

void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges) {
    // 그리드 셀 및 타워 그리기
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float x1, y1, x2, y2;
            cell_rect(r, c, &x1, &y1, &x2, &y2);

            // 그리드 라인
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(100, 100, 100), 1.0f);

            if (grid[r][c].type != TOWER_EMPTY) {
                float cx = (x1 + x2) * 0.5f;
                float cy = (y1 + y2) * 0.5f;

                // 타워 스프라이트 그리기 (MJ의 에셋 사용)
                ALLEGRO_BITMAP* tower_sprite = NULL;
                if (grid[r][c].type == TOWER_ATTACK) tower_sprite = icon_people1;
                else if (grid[r][c].type == TOWER_RESOURCE) tower_sprite = icon_people2;
                else if (grid[r][c].type == TOWER_TANK) tower_sprite = icon_people3;

                if (tower_sprite) {
                    float sw = al_get_bitmap_width(tower_sprite);
                    float sh = al_get_bitmap_height(tower_sprite);
                    float scale = (CELL_W < CELL_H ? CELL_W : CELL_H) * 0.8f / (sw > sh ? sw : sh);
                    float dw = sw * scale, dh = sh * scale;
                    al_draw_scaled_bitmap(tower_sprite, 0, 0, sw, sh,
                        cx - dw / 2, cy - dh / 2, dw, dh, 0);
                }

                // HP 바
                int maxhp = tower_max_hp(grid[r][c].type);
                if (maxhp > 0) {
                    float ratio = (float)grid[r][c].hp / (float)maxhp;
                    float barw = CELL_W - 10.0f;
                    float bx = x1 + 5.0f;
                    float by = y1 + 5.0f;
                    al_draw_filled_rectangle(bx, by, bx + barw, by + 4, al_map_rgb(40, 40, 40));
                    al_draw_filled_rectangle(bx, by, bx + barw * ratio, by + 4, al_map_rgb(220, 120, 120));
                }
            }
        }
    }

    // 적 그리기
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].active) {
            // 적 스프라이트 (졸고 있는 학생)
            if (icon_sleeping) {
                float sw = al_get_bitmap_width(icon_sleeping);
                float sh = al_get_bitmap_height(icon_sleeping);
                float scale = 0.8f;
                float dw = sw * scale, dh = sh * scale;
                al_draw_scaled_bitmap(icon_sleeping, 0, 0, sw, sh,
                    enemies[i].x - dw / 2, enemies[i].y - dh / 2, dw, dh, 0);
            }
            else {
                // 폴백: 사각형
                al_draw_filled_rectangle(
                    enemies[i].x - 8, enemies[i].y - 8,
                    enemies[i].x + 8, enemies[i].y + 8,
                    al_map_rgb(150, 50, 200)
                );
            }

            // 적 HP 바
            float eratio = (float)enemies[i].hp / (float)ENEMY_HP;
            if (eratio < 0.0f) eratio = 0.0f;
            float ex1 = enemies[i].x - 12, ey1 = enemies[i].y - 20;
            float ex2 = enemies[i].x + 12, ey2 = ey1 + 4;
            al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
            al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * eratio, ey2, al_map_rgb(120, 220, 120));
        }
    }

    // 공격 범위 표시
    if (show_ranges) {
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_ATTACK) {
                    float x1, y1, x2, y2;
                    cell_rect(r, c, &x1, &y1, &x2, &y2);
                    float cx = (x1 + x2) * 0.5f;
                    float cy = (y1 + y2) * 0.5f;
                    al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 120), 2.0f);
                }
            }
        }
    }

    // 선택된 셀의 공격 범위
    if (!show_ranges && grid[cursor_row][cursor_col].type == TOWER_ATTACK) {
        float x1, y1, x2, y2;
        cell_rect(cursor_row, cursor_col, &x1, &y1, &x2, &y2);
        float cx = (x1 + x2) * 0.5f;
        float cy = (y1 + y2) * 0.5f;
        al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 150), 2.0f);
    }

    // 커서 표시
    if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
        float cx1, cy1, cx2, cy2;
        cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
        al_draw_rectangle(cx1 + 2, cy1 + 2, cx2 - 2, cy2 - 2, al_map_rgb(255, 255, 0), 3.0f);
    }
}

void game_place_tower(TowerType type, int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    if (grid[row][col].type != TOWER_EMPTY) return;

    int cost = 0;
    if (type == TOWER_ATTACK) cost = ATTACK_TOWER_COST;
    else if (type == TOWER_RESOURCE) cost = RESOURCE_TOWER_COST;
    else if (type == TOWER_TANK) cost = TANK_TOWER_COST;

    if (game_state.caffeine >= cost) {
        game_state.caffeine -= cost;
        grid[row][col].type = type;
        grid[row][col].cooldown = 0;
        grid[row][col].hp = tower_max_hp(type);
    }
}

void game_sell_tower(int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    Tower* cell = &grid[row][col];
    if (cell->type == TOWER_EMPTY) return;

    int refund = 0;
    if (cell->type == TOWER_ATTACK) refund = ATTACK_TOWER_COST / 2;
    else if (cell->type == TOWER_RESOURCE) refund = RESOURCE_TOWER_COST / 2;
    else if (cell->type == TOWER_TANK) refund = TANK_TOWER_COST / 2;

    game_state.caffeine += refund;
    cell->type = TOWER_EMPTY;
    cell->hp = 0;
    cell->cooldown = 0;
}

GameState game_get_state(void) {
    return game_state;
}
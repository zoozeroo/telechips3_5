#include "game.h"
#include "assets.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

// ===== 그리드 레이아웃 =====
#define CELL_W 102
#define CELL_H 102
#define GRID_X 21
#define GRID_Y 186

static Tower grid[GRID_ROWS][GRID_COLS];
static Enemy enemies[MAX_ENEMIES];
static GameState game_state;
static double last_enemy_spawn_time = 0.0;

// "STAGE N" 배너 표시용(3초)
static double stage_banner_until = 0.0;
static int    stage_banner_stage = 0;

// 1초 주기 깜빡임(짝/홀 초)
static inline bool anim_blink_1s(void) { return (((int)al_get_time()) % 2) == 0; }

static inline int tower_max_hp(TowerType t) {
    if (t == TOWER_ATTACK)   return ATTACK_TOWER_HP;
    if (t == TOWER_RESOURCE) return RESOURCE_TOWER_HP;
    if (t == TOWER_TANK)     return TANK_TOWER_HP;
    return 0;
}

static void cell_rect(int row, int col, float* x1, float* y1, float* x2, float* y2) {
    *x1 = GRID_X + col * CELL_W;
    *y1 = GRID_Y + row * CELL_H;
    *x2 = *x1 + CELL_W;
    *y2 = *y1 + CELL_H;
}

static float distancef(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// 설치된 모든 타워 제거 + 코인 환불(설치 비용의 50%)
static void clear_all_towers_and_refund(void) {
    int refund_total = 0;

    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            Tower* t = &grid[r][c];
            if (t->type == TOWER_EMPTY) continue;

            int cost = 0;
            if (t->type == TOWER_ATTACK)        cost = ATTACK_TOWER_COST;
            else if (t->type == TOWER_RESOURCE) cost = RESOURCE_TOWER_COST;
            else if (t->type == TOWER_TANK)     cost = TANK_TOWER_COST;

            refund_total += cost / 2;       // 50% 환불
            t->type = TOWER_EMPTY;
            t->hp = 0;
            t->cooldown = 0.0f;
        }
    }
    game_state.caffeine += refund_total;
}

static void spawn_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].x = GRID_X + GRID_COLS * CELL_W + 20;  // 오른쪽 화면 밖에서 등장
            enemies[i].y = GRID_Y + (rand() % (int)(GRID_ROWS * CELL_H));
            enemies[i].hp = ENEMY_HP;
            enemies[i].atk_cooldown = 0.0f;
            break;
        }
    }
}

static ALLEGRO_BITMAP* tower_anim_frame(TowerType t) {
    bool blink = anim_blink_1s();
    switch (t) {
    case TOWER_ATTACK:   // A
        return blink ? (icon_people1 ? icon_people1 : icon_sleeping)
            : (icon_people1_1 ? icon_people1_1 : icon_people1);
    case TOWER_RESOURCE: // S
        return blink ? (icon_people2 ? icon_people2 : icon_sleeping)
            : (icon_people2_1 ? icon_people2_1 : icon_people2);
    case TOWER_TANK:     // D
        return blink ? (icon_people3 ? icon_people3 : icon_sleeping)
            : (icon_people3_1 ? icon_people3_1 : icon_people3);
    default: return NULL;
    }
}

void game_init(void) {
    srand((unsigned)time(NULL));

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

    game_state.caffeine = STARTING_CAFFEINE; // ★ 시작 코인
    game_state.lives = 5;                 // 라이프 5
    game_state.stage = 1;
    game_state.stage_kills = 0;
    game_state.cleared = false;
    game_state.game_over = false;

    stage_banner_until = al_get_time() + 3.0; // STAGE 1 배너 3초
    stage_banner_stage = 1;

    last_enemy_spawn_time = stage_banner_until; // 배너 끝나고 스폰 시작
}

void game_reset(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i].active = false;
    game_state.game_over = false;
    game_state.cleared = false;
    // 배너/스폰 타이밍은 game_init에서 세팅
}

static void on_enemy_killed(void) {
    game_state.stage_kills++;
    if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
        // 화면에 남은 적 제거
        for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i].active = false;

        // 마지막 스테이지면 게임 클리어
        if (game_state.stage >= MAX_STAGES) {
            game_state.cleared = true;
            return;
        }

        // 다음 스테이지 진입 전: 타워 초기화 + 50% 환불
        clear_all_towers_and_refund();

        // ===== 스테이지 넘김 =====
        game_state.stage++;
        game_state.stage_kills = 0;

        // ★★ 다음 스테이지 시작 코인을 1스테이지 시작 코인과 동일하게 강제 세팅
        game_state.caffeine = STARTING_CAFFEINE;

        // 3초 배너 셋업 + 그 이후부터 스폰
        stage_banner_stage = game_state.stage;
        stage_banner_until = al_get_time() + 3.0;
        last_enemy_spawn_time = stage_banner_until;
    }
}

void game_update(float dt) {
    if (game_state.cleared || game_state.game_over) return;

    // ===== 타워 로직 =====
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float tcx = GRID_X + c * CELL_W + CELL_W * 0.5f;
            float tcy = GRID_Y + r * CELL_H + CELL_H * 0.5f;

            if (grid[r][c].cooldown > 0.0f) grid[r][c].cooldown -= dt;

            if (grid[r][c].type == TOWER_ATTACK && grid[r][c].cooldown <= 0.0f) {
                Enemy* target = NULL;
                float best = ATTACK_TOWER_RANGE;
                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (!enemies[i].active) continue;
                    float d = distancef(tcx, tcy, enemies[i].x, enemies[i].y);
                    if (d < best) { best = d; target = &enemies[i]; }
                }
                if (target) {
                    target->hp -= ATTACK_TOWER_DAMAGE;
                    if (target->hp <= 0) {
                        target->active = false;
                        game_state.caffeine += 20; // 처치 보상
                        on_enemy_killed();
                    }
                    grid[r][c].cooldown = ATTACK_TOWER_COOLDOWN;
                }
            }
            else if (grid[r][c].type == TOWER_RESOURCE && grid[r][c].cooldown <= 0.0f) {
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                grid[r][c].cooldown = RESOURCE_TOWER_COOLDOWN;
            }
            // TOWER_TANK는 현재 수동기능(탱킹)만. 필요 시 능동 스킬 추가 가능.
        }
    }

    // ===== 적 로직 =====
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (enemies[i].atk_cooldown > 0.0f) enemies[i].atk_cooldown -= dt;

        // 가장 가까운 타워를 근접 사정거리 내에서 찾기
        Tower* target_cell = NULL;
        float best_d = ENEMY_ATTACK_RANGE;

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_EMPTY) continue;
                float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f;
                float cy = (y1 + y2) * 0.5f;
                float d = distancef(enemies[i].x, enemies[i].y, cx, cy);
                if (d < best_d) { best_d = d; target_cell = &grid[r][c]; }
            }
        }

        if (target_cell) {
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
            // 타겟 없으면 왼쪽으로 이동
            enemies[i].x -= ENEMY_SPEED;
            if (enemies[i].x <= GRID_X - 20) {
                enemies[i].active = false;
                game_state.lives--;
                if (game_state.lives <= 0) game_state.game_over = true;
            }
        }
    }

    // ===== 적 스폰(배너 중엔 스폰 지연) =====
    if (!game_state.cleared &&
        game_state.stage_kills < KILLS_TO_ADVANCE &&
        (al_get_time() - last_enemy_spawn_time > 2.0))
    {
        spawn_enemy();
        last_enemy_spawn_time = al_get_time();
    }
}

void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges) {
    // ===== 그리드 + 타워 =====
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(100, 100, 100), 1.0f);

            if (grid[r][c].type != TOWER_EMPTY) {
                float cx = (x1 + x2) * 0.5f;
                float cy = (y1 + y2) * 0.5f;

                // 깜빡임 애니메이션 프레임
                ALLEGRO_BITMAP* bmp = tower_anim_frame(grid[r][c].type);
                if (bmp) {
                    float sw = (float)al_get_bitmap_width(bmp);
                    float sh = (float)al_get_bitmap_height(bmp);
                    float base = (CELL_W < CELL_H ? CELL_W : CELL_H) * 0.8f;
                    float scale = base / ((sw > sh) ? sw : sh);
                    float dw = sw * scale, dh = sh * scale;
                    al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, cx - dw * 0.5f, cy - dh * 0.5f, dw, dh, 0);
                }

                // HP 바
                int maxhp = tower_max_hp(grid[r][c].type);
                if (maxhp > 0) {
                    float ratio = (float)grid[r][c].hp / (float)maxhp;
                    if (ratio < 0) ratio = 0;
                    float barw = CELL_W - 10.0f;
                    float bx = x1 + 5.0f, by = y1 + 5.0f;
                    al_draw_filled_rectangle(bx, by, bx + barw, by + 4, al_map_rgb(40, 40, 40));
                    al_draw_filled_rectangle(bx, by, bx + barw * ratio, by + 4, al_map_rgb(220, 120, 120));
                }
            }
        }
    }

    // ===== 적 =====
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (icon_virus1) {
            float sw = (float)al_get_bitmap_width(icon_virus1);
            float sh = (float)al_get_bitmap_height(icon_virus1);
            float dw = sw * 2.0f, dh = sh * 2.0f;
            al_draw_scaled_bitmap(icon_virus1, 0, 0, sw, sh,
                enemies[i].x - dw * 0.5f, enemies[i].y - dh * 0.5f, dw, dh, 0);
        }
        else {
            al_draw_filled_rectangle(enemies[i].x - 8, enemies[i].y - 8,
                enemies[i].x + 8, enemies[i].y + 8, al_map_rgb(150, 50, 200));
        }

        // HP 바
        float er = (float)enemies[i].hp / (float)ENEMY_HP; if (er < 0) er = 0;
        float ex1 = enemies[i].x - 12, ey1 = enemies[i].y - 20, ex2 = enemies[i].x + 12, ey2 = ey1 + 4;
        al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
        al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * er, ey2, al_map_rgb(120, 220, 120));
    }

    // ===== 공격 범위 표시 =====
    if (show_ranges) {
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type != TOWER_ATTACK) continue;
                float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
                al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 120), 2.0f);
            }
        }
    }
    else {
        if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
            if (grid[cursor_row][cursor_col].type == TOWER_ATTACK) {
                float x1, y1, x2, y2; cell_rect(cursor_row, cursor_col, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
                al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 150), 2.0f);
            }
        }
    }

    // ===== 커서 표시 =====
    if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
        float cx1, cy1, cx2, cy2; cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
        al_draw_rectangle(cx1 + 2, cy1 + 2, cx2 - 2, cy2 - 2, al_map_rgb(255, 255, 0), 3.0f);
    }

    // ===== STAGE 배너(3초) =====
    if (al_get_time() < stage_banner_until && stage_banner_stage > 0) {
        float overlay_w = (float)W * 0.6f;
        float overlay_h = 120.0f;
        float ox = (W - overlay_w) * 0.5f;
        float oy = (H - overlay_h) * 0.25f;

        al_draw_filled_rounded_rectangle(ox, oy, ox + overlay_w, oy + overlay_h, 12, 12,
            al_map_rgba(0, 0, 0, 160));
        al_draw_rounded_rectangle(ox, oy, ox + overlay_w, oy + overlay_h, 12, 12,
            al_map_rgb(255, 255, 255), 2.0f);

        if (font_title) {
            char msg[64];
            snprintf(msg, sizeof msg, "STAGE %d", stage_banner_stage);
            al_draw_text(font_title, al_map_rgb(255, 255, 255),
                (float)W * 0.5f, oy + 35.0f, ALLEGRO_ALIGN_CENTER, msg);
        }
    }
}

void game_place_tower(TowerType type, int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    if (grid[row][col].type != TOWER_EMPTY) return;

    int cost = 0;
    if (type == TOWER_ATTACK)        cost = ATTACK_TOWER_COST;
    else if (type == TOWER_RESOURCE) cost = RESOURCE_TOWER_COST;
    else if (type == TOWER_TANK)     cost = TANK_TOWER_COST;

    if (game_state.caffeine >= cost) {
        game_state.caffeine -= cost;
        grid[row][col].type = type;
        grid[row][col].cooldown = 0.0f;
        grid[row][col].hp = tower_max_hp(type);
    }
}

void game_sell_tower(int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    Tower* t = &grid[row][col];
    if (t->type == TOWER_EMPTY) return;

    int refund = 0;
    if (t->type == TOWER_ATTACK)        refund = ATTACK_TOWER_COST / 2;
    else if (t->type == TOWER_RESOURCE) refund = RESOURCE_TOWER_COST / 2;
    else if (t->type == TOWER_TANK)     refund = TANK_TOWER_COST / 2;

    game_state.caffeine += refund;
    t->type = TOWER_EMPTY;
    t->hp = 0;
    t->cooldown = 0.0f;
}

GameState game_get_state(void) {
    return game_state;
}

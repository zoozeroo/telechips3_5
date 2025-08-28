#include "game.h"
#include "assets.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>   // 배너 텍스트 출력용

// ───────── 그리드 레이아웃 ─────────
#define CELL_W 102
#define CELL_H 102
#define GRID_X 21
#define GRID_Y 186

static Tower  grid[GRID_ROWS][GRID_COLS];
static Enemy  enemies[MAX_ENEMIES];
static Bullet bullets[MAX_BULLETS];
static GameState game_state;

static double last_enemy_spawn_time = 0.0;

// ── 스테이지 배너 상태 (2초 동안 게임 일시 정지) ──
static bool   stage_banner_active = false;
static double stage_banner_until = 0.0;
static int    stage_banner_stage = 0;

// ───────── 유틸 ─────────
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

static inline bool anim_blink_1s(void) { return (((int)al_get_time()) % 2) == 0; }

static ALLEGRO_BITMAP* tower_anim_frame(TowerType t) {
    bool blink = anim_blink_1s();
    switch (t) {
    case TOWER_EMPTY:    return icon_sleeping;
    case TOWER_ATTACK:   return blink ? (icon_people1 ? icon_people1 : icon_sleeping)
        : (icon_people1_1 ? icon_people1_1 : icon_people1);
    case TOWER_RESOURCE: return blink ? (icon_people2 ? icon_people2 : icon_sleeping)
        : (icon_people2_1 ? icon_people2_1 : icon_people2);
    case TOWER_TANK:     return blink ? (icon_people3 ? icon_people3 : icon_sleeping)
        : (icon_people3_1 ? icon_people3_1 : icon_people3);
    default:             return icon_sleeping;
    }
}

// ───────── 초기화 헬퍼 ─────────
static void clear_all_enemies(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i].active = false;
}
static void clear_all_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; ++i) bullets[i].active = false;
}
static void clear_all_towers(void) {
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c].type = TOWER_EMPTY;
            grid[r][c].hp = 0;
            grid[r][c].cooldown = 0.0f;
        }
    }
}
static void begin_stage_banner(int stage) {
    stage_banner_stage = stage;
    stage_banner_active = true;
    stage_banner_until = al_get_time() + 2.0;    // ★ 2초
    last_enemy_spawn_time = stage_banner_until;   // 배너 끝난 직후부터 스폰 타이밍 카운트
}

// ───────── 적/총알 스폰 ─────────
static void spawn_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].x = GRID_X + GRID_COLS * CELL_W + 20;
            enemies[i].y = GRID_Y + (rand() % (int)(GRID_ROWS * CELL_H));
            enemies[i].hp = ENEMY_HP;
            enemies[i].atk_cooldown = 0.0f;
            break;
        }
    }
}

static int spawn_bullet(float sx, float sy, int enemy_index) {
    if (enemy_index < 0 || enemy_index >= MAX_ENEMIES || !enemies[enemy_index].active) return -1;

    float tx = enemies[enemy_index].x, ty = enemies[enemy_index].y;
    float dx = tx - sx, dy = ty - sy;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1e-4f) { dx = 1.0f; dy = 0.0f; len = 1.0f; }
    dx /= len; dy /= len;

    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].x = sx; bullets[i].y = sy;
            bullets[i].vx = dx * BULLET_SPEED;
            bullets[i].vy = dy * BULLET_SPEED;
            bullets[i].image_type = rand() % 3;
            return i;
        }
    }
    return -1;
}

static bool circle_rect_overlap(float cx, float cy, float r, float rx1, float ry1, float rx2, float ry2) {
    float nnx = (cx < rx1) ? rx1 : (cx > rx2) ? rx2 : cx;
    float nny = (cy < ry1) ? ry1 : (cy > ry2) ? ry2 : cy;
    float dx = cx - nnx, dy = cy - nny;
    return (dx * dx + dy * dy) <= r * r;
}

// ───────── 콜백: 적 처치 ─────────
static void on_enemy_killed(void) {
    game_state.stage_kills++;
    if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
        // 마지막 스테이지면 클리어
        if (game_state.stage >= MAX_STAGES) {
            game_state.cleared = true;
            clear_all_enemies();
            clear_all_bullets();
            return;
        }

        // 다음 스테이지 준비: 적/총알/타워 정리, 재화 초기화, 배너 2초
        clear_all_enemies();
        clear_all_bullets();
        clear_all_towers();

        game_state.stage++;
        game_state.stage_kills = 0;
        game_state.caffeine = STARTING_CAFFEINE;   // ★ 재화 초기화

        begin_stage_banner(game_state.stage);
    }
}

// ───────── 퍼블릭 API ─────────
void game_init(void) {
    srand((unsigned)time(NULL));

    clear_all_towers();
    clear_all_enemies();
    clear_all_bullets();

    game_state.caffeine = STARTING_CAFFEINE; // ★ 시작 재화
    game_state.lives = 5;
    game_state.stage = 1;
    game_state.stage_kills = 0;
    game_state.cleared = false;
    game_state.game_over = false;

    begin_stage_banner(1); // ★ 스테이지 1 배너 2초
}

void game_reset(void) {
    clear_all_enemies();
    clear_all_bullets();
    game_state.game_over = false;
    game_state.cleared = false;
}

// 메인 업데이트
void game_update(float dt) {
    if (game_state.cleared || game_state.game_over) return;

    // ── 배너 중엔 완전 정지 ──
    if (stage_banner_active) {
        if (al_get_time() < stage_banner_until) return;
        stage_banner_active = false;               // 배너 종료 → 그때부터 정상 진행
        last_enemy_spawn_time = al_get_time();
    }

    // ── 타워 로직 ──
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float tcx = GRID_X + c * CELL_W + CELL_W * 0.5f;
            float tcy = GRID_Y + r * CELL_H + CELL_H * 0.5f;

            if (grid[r][c].cooldown > 0.0f) grid[r][c].cooldown -= dt;

            if (grid[r][c].type == TOWER_ATTACK && grid[r][c].cooldown <= 0.0f) {
                Enemy* target = NULL; int target_index = -1;
                float best = ATTACK_TOWER_RANGE;
                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (!enemies[i].active) continue;
                    float d = distancef(tcx, tcy, enemies[i].x, enemies[i].y);
                    if (d < best) { best = d; target = &enemies[i]; target_index = i; }
                }
                if (target) {
                    spawn_bullet(tcx, tcy, target_index);
                    grid[r][c].cooldown = ATTACK_TOWER_COOLDOWN;
                }
            }
            else if (grid[r][c].type == TOWER_RESOURCE && grid[r][c].cooldown <= 0.0f) {
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                grid[r][c].cooldown = RESOURCE_TOWER_COOLDOWN;
            }
        }
    }

    // ── 적 로직 ──
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (enemies[i].atk_cooldown > 0.0f) enemies[i].atk_cooldown -= dt;

        Tower* target_cell = NULL;
        float best_d = ENEMY_ATTACK_RANGE;

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_EMPTY) continue;
                float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
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
            enemies[i].x -= ENEMY_SPEED;
            if (enemies[i].x <= GRID_X - 20) {
                enemies[i].active = false;
                game_state.lives--;
                if (game_state.lives <= 0) game_state.game_over = true;
            }
        }
    }

    // ── 적 스폰(2초 간격) ──
    if (game_state.stage_kills < KILLS_TO_ADVANCE &&
        (al_get_time() - last_enemy_spawn_time > 2.0)) {
        spawn_enemy();
        last_enemy_spawn_time = al_get_time();
    }

    // ── 총알 업데이트 ──
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet* b = &bullets[i];
        if (!b->active) continue;

        b->x += b->vx * dt;
        b->y += b->vy * dt;

        float grid_right = GRID_X + GRID_COLS * CELL_W + 50;
        float grid_bottom = GRID_Y + GRID_ROWS * CELL_H + 50;
        if (b->x < -20 || b->x > grid_right || b->y < -20 || b->y > grid_bottom) {
            b->active = false; continue;
        }

        for (int e = 0; e < MAX_ENEMIES; ++e) {
            if (!enemies[e].active) continue;
            float ex = enemies[e].x, ey = enemies[e].y;
            if (circle_rect_overlap(b->x, b->y, BULLET_RADIUS, ex - 8, ey - 8, ex + 8, ey + 8)) {
                enemies[e].hp -= ATTACK_TOWER_DAMAGE;
                if (enemies[e].hp <= 0) {
                    enemies[e].active = false;
                    game_state.caffeine += 20;
                    on_enemy_killed();
                }
                b->active = false;
                break;
            }
        }
    }
}

// 메인 그리기
void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges) {
    // ── 그리드 / 타워 ──
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(100, 100, 100), 1.0f);

            float cx = (x1 + x2) * 0.5f;
            float cy = (y1 + y2) * 0.5f;

            ALLEGRO_BITMAP* tower_sprite = tower_anim_frame(grid[r][c].type);
            if (tower_sprite) {
                float sw = (float)al_get_bitmap_width(tower_sprite);
                float sh = (float)al_get_bitmap_height(tower_sprite);
                float base = (CELL_W < CELL_H ? CELL_W : CELL_H) * 0.8f;
                float scale = base / ((sw > sh) ? sw : sh);
                float dw = sw * scale, dh = sh * scale;
                al_draw_scaled_bitmap(tower_sprite, 0, 0, sw, sh,
                    cx - dw / 2, cy - dh / 2, dw, dh, 0);
            }

            if (grid[r][c].type != TOWER_EMPTY) {
                int maxhp = tower_max_hp(grid[r][c].type);
                if (maxhp > 0) {
                    float ratio = (float)grid[r][c].hp / (float)maxhp;
                    float barw = CELL_W - 10.0f;
                    float bx = x1 + 5.0f, by = y1 + 5.0f;
                    al_draw_filled_rectangle(bx, by, bx + barw, by + 4, al_map_rgb(40, 40, 40));
                    al_draw_filled_rectangle(bx, by, bx + barw * ratio, by + 4, al_map_rgb(220, 120, 120));
                }
            }
        }
    }

    // ── 적 ──
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (icon_virus1) {
            float sw = (float)al_get_bitmap_width(icon_virus1);
            float sh = (float)al_get_bitmap_height(icon_virus1);
            float dw = sw * 2.0f, dh = sh * 2.0f;
            al_draw_scaled_bitmap(icon_virus1, 0, 0, sw, sh,
                enemies[i].x - dw / 2, enemies[i].y - dh / 2, dw, dh, 0);
        }
        else {
            al_draw_filled_rectangle(enemies[i].x - 8, enemies[i].y - 8,
                enemies[i].x + 8, enemies[i].y + 8, al_map_rgb(150, 50, 200));
        }

        float er = (float)enemies[i].hp / (float)ENEMY_HP; if (er < 0) er = 0;
        float ex1 = enemies[i].x - 12, ey1 = enemies[i].y - 20, ex2 = enemies[i].x + 12, ey2 = ey1 + 4;
        al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
        al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * er, ey2, al_map_rgb(120, 220, 120));
    }

    // ── 공격 범위 ──
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

    // ── 커서 ──
    if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
        float cx1, cy1, cx2, cy2; cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
        al_draw_rectangle(cx1 + 2, cy1 + 2, cx2 - 2, cy2 - 2, al_map_rgb(255, 255, 0), 3.0f);
    }

    // ── 총알 ──
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;
        ALLEGRO_BITMAP* bullet_img = NULL;
    switch (bullets[i].image_type) { case 0: bullet_img = bullet_1; break; case 1: bullet_img = bullet_2; break; default: bullet_img = bullet_3; break; }
                                           if (bullet_img) {
                                               float iw = (float)al_get_bitmap_width(bullet_img);
                                               float ih = (float)al_get_bitmap_height(bullet_img);
                                               float scale = 3.0f;
                                               al_draw_scaled_bitmap(bullet_img, 0, 0, iw, ih, bullets[i].x - (iw * 0.5f), bullets[i].y - (ih * 0.5f), iw * scale, ih * scale, 0);
                                           }
                                           else {
                                               al_draw_filled_circle(bullets[i].x, bullets[i].y, BULLET_RADIUS, al_map_rgb(255, 255, 255));
                                           }
    }

    // ── STAGE 배너(2초) ──
    if (stage_banner_active) {
        float overlay_w = (float)W * 0.6f;
        float overlay_h = 120.0f;
        float ox = (W - overlay_w) * 0.5f;
        float oy = (H - overlay_h) * 0.25f;

        al_draw_filled_rounded_rectangle(ox, oy, ox + overlay_w, oy + overlay_h, 12, 12, al_map_rgba(0, 0, 0, 160));
        al_draw_rounded_rectangle(ox, oy, ox + overlay_w, oy + overlay_h, 12, 12, al_map_rgb(255, 255, 255), 2.0f);

        if (font_title) {
            char msg[64]; snprintf(msg, sizeof msg, "STAGE %d", stage_banner_stage);
            al_draw_text(font_title, al_map_rgb(255, 255, 255), (float)W * 0.5f, oy + 35.0f, ALLEGRO_ALIGN_CENTER, msg);
        }
    }
}

// ── 설치 / 판매 ──
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

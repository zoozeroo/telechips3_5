#include "game.h"
#include "assets.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>                  // snprintf 사용
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

// ───────── 그리드 레이아웃 ─────────
#define CELL_W 102
#define CELL_H 102
#define GRID_X 21
#define GRID_Y 186

static Tower  grid[GRID_ROWS][GRID_COLS];
static Enemy  enemies[MAX_ENEMIES];
static Bullet bullets[MAX_BULLETS];
static GameState game_state;

// 스폰 타이밍
static double last_enemy_spawn_time = 0.0;

// ── 폭발 이펙트(1초 동안 icon_bombeffect1→2→3) ──
typedef struct {
    bool active;
    float x, y;
    double t0;       // 시작 시각
    double dur;      // 지속 시간(1.0s)
} BombFx;
#define MAX_BOMB_FX 32
static BombFx g_bomb_fx[MAX_BOMB_FX];

// ── 스테이지 배너(요청 구현 유지) ──
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

static ALLEGRO_BITMAP* tower_anim_frame(const Tower* t) {
    // 얼음 상태면 종류별로 얼음 아이콘 고정 표시
    if (t->freeze_stacks > 0) {
        switch (t->type) {
        case TOWER_ATTACK:   return icon_frozen1;
        case TOWER_RESOURCE: return icon_frozen2;
        case TOWER_TANK:     return icon_frozen3;  // ★ 탱커도 확실히 얼음 아이콘 적용
        default:             return icon_sleeping;
        }
    }
    // 평상시 애니메이션
    bool blink = anim_blink_1s();
    switch (t->type) {
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

static void clear_all_enemies(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
        enemies[i].x = enemies[i].y = 0.0f;
        enemies[i].hp = 0;
        enemies[i].atk_cooldown = 0.0f;
        enemies[i].type = ET_FAST;
        enemies[i].speed = 0.0f;
        enemies[i].fuse_timer = 0.0f;
        enemies[i].freeze_link_count = 0;
    }
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
            grid[r][c].freeze_stacks = 0;
        }
    }
}
static void clear_all_fx(void) {
    for (int i = 0; i < MAX_BOMB_FX; ++i) g_bomb_fx[i].active = false;
}

static void begin_stage_banner(int stage) {
    stage_banner_stage = stage;
    stage_banner_active = true;
    stage_banner_until = al_get_time() + 2.0;
    last_enemy_spawn_time = stage_banner_until;
}

static void spawn_bomb_fx(float x, float y) {
    for (int i = 0; i < MAX_BOMB_FX; ++i) {
        if (!g_bomb_fx[i].active) {
            g_bomb_fx[i].active = true;
            g_bomb_fx[i].x = x;
            g_bomb_fx[i].y = y;
            g_bomb_fx[i].t0 = al_get_time();
            g_bomb_fx[i].dur = 1.0; // 1초
            return;
        }
    }
}

// ───────── 스폰 ─────────
static void spawn_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            Enemy* e = &enemies[i];
            e->active = true;

            // 오른쪽 가장자리 + 랜덤 행 중앙
            float right_edge = GRID_X + GRID_COLS * CELL_W;
            e->x = right_edge + 20.0f;
            int row = rand() % GRID_ROWS;
            float x1, y1, x2, y2;
            cell_rect(row, 0, &x1, &y1, &x2, &y2);
            e->y = 0.5f * (y1 + y2);

            // 적 출현 타입 확률 (원하면 조절)
            int r = rand() % 100;
            if (r < 30)       e->type = ET_TANK;
            else if (r < 80)  e->type = ET_FAST;
            else if (r < 90)  e->type = ET_BOMBER;
            else              e->type = ET_FREEZER;

            e->atk_cooldown = 0.0f;
            e->fuse_timer = 0.0f;
            e->freeze_link_count = 0;

            switch (e->type) {
            case ET_FAST:   e->hp = FAST_HP;   e->speed = FAST_SPEED;   break;
            case ET_TANK:   e->hp = TANK_HP;   e->speed = TANK_SPEED;   break;
            case ET_BOMBER: e->hp = BOMB_HP;   e->speed = BOMB_SPEED;   break;
            case ET_FREEZER:e->hp = FREEZER_HP; e->speed = FREEZER_SPEED; break;
            }
            break;
        }
    }
}

// ───────── 스테이트 변경/콜백 ─────────
static void on_enemy_killed(Enemy* e) {
    // FREEZER가 얼려둔 타워 해제
    if (e->type == ET_FREEZER) {
        for (int i = 0; i < e->freeze_link_count; ++i) {
            int r = e->freeze_links[i].r;
            int c = e->freeze_links[i].c;
            if (r < 0 || r >= GRID_ROWS || c < 0 || c >= GRID_COLS) continue;
            if (grid[r][c].freeze_stacks > 0) grid[r][c].freeze_stacks--;
        }
    }

    game_state.stage_kills++;
    if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
        // 모든 적 제거
        for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i].active = false;

        if (game_state.stage >= MAX_STAGES) { game_state.cleared = true; return; }

        // 다음 스테이지: 타워/총알/적 초기화, 재화 리셋, 배너
        clear_all_bullets();
        clear_all_towers();
        game_state.stage++;
        game_state.stage_kills = 0;
        game_state.caffeine = STARTING_CAFFEINE;
        begin_stage_banner(game_state.stage);
    }
}

// ───────── 동작 구현(폭발/얼림) ─────────
static void explode_bomb(Enemy* b) {
    // 이펙트 표시
    spawn_bomb_fx(b->x, b->y);

    // 반경 내 타워 HP를 절반으로
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            if (grid[r][c].type == TOWER_EMPTY) continue;
            float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
            float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
            if (distancef(b->x, b->y, cx, cy) <= BOMB_RADIUS) {
                grid[r][c].hp = grid[r][c].hp / 2;     // 50% 감소
                if (grid[r][c].hp <= 0) {
                    grid[r][c].type = TOWER_EMPTY;
                    grid[r][c].cooldown = 0.0f;
                    grid[r][c].freeze_stacks = 0;
                }
            }
        }
    }

    // 스스로 사망 처리
    b->active = false;
    on_enemy_killed(b);
}

static bool freezer_has_link(const Enemy* frz, int r, int c) {
    for (int i = 0; i < frz->freeze_link_count; ++i) {
        if (frz->freeze_links[i].r == r && frz->freeze_links[i].c == c) return true;
    }
    return false;
}
static void freezer_touch_tower(Enemy* frz, int r, int c) {
    if (r < 0 || r >= GRID_ROWS || c < 0 || c >= GRID_COLS) return;
    Tower* t = &grid[r][c];
    if (t->type == TOWER_EMPTY) return;

    // 이미 연결돼 있으면 중복 추가하지 않음
    if (!freezer_has_link(frz, r, c)) {
        if (frz->freeze_link_count < FREEZER_MAX_LINKS) {
            frz->freeze_links[frz->freeze_link_count].r = r;
            frz->freeze_links[frz->freeze_link_count].c = c;
            frz->freeze_link_count++;
        }
        // 타입과 무관하게 전부 얼림 스택 +1 (탱커도 아이콘만 얼림으로)
        t->freeze_stacks++;
    }
}

// ───────── 퍼블릭 API ─────────
void game_init(void) {
    srand((unsigned)time(NULL));
    clear_all_towers();
    clear_all_enemies();
    clear_all_bullets();
    clear_all_fx();

    game_state.caffeine = STARTING_CAFFEINE;
    game_state.lives = 5;
    game_state.stage = 1;
    game_state.stage_kills = 0;
    game_state.cleared = false;
    game_state.game_over = false;

    begin_stage_banner(1);
}

void game_reset(void) {
    clear_all_enemies();
    clear_all_bullets();
    clear_all_fx();
    for (int r = 0; r < GRID_ROWS; ++r)
        for (int c = 0; c < GRID_COLS; ++c)
            grid[r][c].freeze_stacks = 0;
    game_state.game_over = false;
    game_state.cleared = false;
}

// ───────── 업데이트 ─────────
void game_update(float dt) {
    if (game_state.cleared || game_state.game_over) return;

    // 스테이지 배너 중에는 정지
    if (stage_banner_active) {
        if (al_get_time() < stage_banner_until) return;
        stage_banner_active = false;
        last_enemy_spawn_time = al_get_time();
    }

    // 타워 로직
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            Tower* t = &grid[r][c];
            float tcx = GRID_X + c * CELL_W + CELL_W * 0.5f;
            float tcy = GRID_Y + r * CELL_H + CELL_H * 0.5f;

            if (t->cooldown > 0.0f) t->cooldown -= dt;

            // 얼려 있으면 일절 동작 안 함
            if (t->freeze_stacks > 0) continue;

            if (t->type == TOWER_ATTACK && t->cooldown <= 0.0f) {
                Enemy* target = NULL; int target_idx = -1;
                float best = ATTACK_TOWER_RANGE;
                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (!enemies[i].active) continue;
                    float d = distancef(tcx, tcy, enemies[i].x, enemies[i].y);
                    if (d < best) { best = d; target = &enemies[i]; target_idx = i; }
                }
                if (target) {
                    // 총알 발사
                    float tx = target->x, ty = target->y;
                    float dx = tx - tcx, dy = ty - tcy;
                    float len = sqrtf(dx * dx + dy * dy); if (len < 1e-4f) len = 1.0f;
                    dx /= len; dy /= len;
                    for (int i = 0; i < MAX_BULLETS; ++i) if (!bullets[i].active) {
                        bullets[i].active = true;
                        bullets[i].x = tcx; bullets[i].y = tcy;
                        bullets[i].vx = dx * BULLET_SPEED;
                        bullets[i].vy = dy * BULLET_SPEED;
                        bullets[i].image_type = rand() % 3;
                        break;
                    }
                    t->cooldown = ATTACK_TOWER_COOLDOWN;
                }
            }
            else if (t->type == TOWER_RESOURCE && t->cooldown <= 0.0f) {
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                t->cooldown = RESOURCE_TOWER_COOLDOWN;
            }
        }
    }

    // 적 로직
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy* e = &enemies[i];
        if (!e->active) continue;

        // 자폭 타이머
        if (e->type == ET_BOMBER && e->fuse_timer > 0.0f) {
            e->fuse_timer -= dt;
            if (e->fuse_timer <= 0.0f) {
                explode_bomb(e);
                continue;
            }
        }

        if (e->atk_cooldown > 0.0f) e->atk_cooldown -= dt;

        // 주변 타겟 타워 탐색
        Tower* target_cell = NULL; int tgt_r = -1, tgt_c = -1;
        float best_d = ENEMY_ATTACK_RANGE;

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                if (grid[r][c].type == TOWER_EMPTY) continue;
                float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
                float d = distancef(e->x, e->y, cx, cy);
                if (d < best_d) { best_d = d; target_cell = &grid[r][c]; tgt_r = r; tgt_c = c; }
            }
        }

        if (target_cell) {
            // ★ 타입별 충돌 처리
            if (e->type == ET_BOMBER) {
                // 타워와 접촉하면 2초 뒤 자폭 (이미 카운트중이면 유지)
                if (e->fuse_timer <= 0.0f) e->fuse_timer = BOMB_ARM_TIME;
            }
            else if (e->type == ET_FREEZER) {
                // 접촉한 타워 얼리기(탱커 포함 전부)
                freezer_touch_tower(e, tgt_r, tgt_c);
            }

            // 일반 공격(탱커/빠른 적 등)
            if (e->atk_cooldown <= 0.0f) {
                target_cell->hp -= ENEMY_ATTACK_DAMAGE;
                e->atk_cooldown = ENEMY_ATTACK_COOLDOWN;
                if (target_cell->hp <= 0) {
                    target_cell->type = TOWER_EMPTY;
                    target_cell->cooldown = 0.0f;
                    target_cell->freeze_stacks = 0;
                }
            }
        }
        else {
            // 타워가 가까이 없으면 이동
            e->x -= e->speed * dt;
            if (e->x <= GRID_X - 20) {
                // 화면 통과 → 라이프 감소, FREEZER 얼음 해제
                if (e->type == ET_FREEZER) {
                    for (int k = 0; k < e->freeze_link_count; ++k) {
                        int rr = e->freeze_links[k].r, cc = e->freeze_links[k].c;
                        if (rr >= 0 && rr < GRID_ROWS && cc >= 0 && cc < GRID_COLS) {
                            if (grid[rr][cc].freeze_stacks > 0) grid[rr][cc].freeze_stacks--;
                        }
                    }
                }
                e->active = false;
                game_state.lives--;
                if (game_state.lives <= 0) game_state.game_over = true;
            }
        }
    }

    // 적 스폰 (간단: 2초마다 1마리 — 이전에 맞춘 스폰표가 있으면 그대로 쓰세요)
    if (game_state.stage_kills < KILLS_TO_ADVANCE &&
        (al_get_time() - last_enemy_spawn_time > 2.0)) {
        spawn_enemy();
        last_enemy_spawn_time = al_get_time();
    }

    // 총알 업데이트/피격
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet* b = &bullets[i];
        if (!b->active) continue;

        b->x += b->vx * dt;
        b->y += b->vy * dt;

        float grid_right = GRID_X + GRID_COLS * CELL_W + 50;
        float grid_bottom = GRID_Y + GRID_ROWS * CELL_H + 50;
        if (b->x < -20 || b->x > grid_right || b->y < -20 || b->y > grid_bottom) { b->active = false; continue; }

        for (int eidx = 0; eidx < MAX_ENEMIES; ++eidx) {
            Enemy* e = &enemies[eidx];
            if (!e->active) continue;

            // 간단한 히트박스
            float ex1 = e->x - 8, ey1 = e->y - 8, ex2 = e->x + 8, ey2 = e->y + 8;
            float cx = b->x, cy = b->y, rr = BULLET_RADIUS;
            float nnx = (cx < ex1) ? ex1 : (cx > ex2) ? ex2 : cx;
            float nny = (cy < ey1) ? ey1 : (cy > ey2) ? ey2 : cy;
            float dx = cx - nnx, dy = cy - nny;
            if ((dx * dx + dy * dy) <= rr * rr) {
                b->active = false;

                e->hp -= ATTACK_TOWER_DAMAGE;
                if (e->hp <= 0) {
                    if (e->type == ET_BOMBER) {
                        // 총알로 죽어도 폭발은 발생
                        explode_bomb(e);
                    }
                    else {
                        e->active = false;
                        game_state.caffeine += 20; // 기존 보상 유지
                        on_enemy_killed(e);
                    }
                }
                break;
            }
        }
    }

    // 폭발 이펙트 수명 관리
    for (int i = 0; i < MAX_BOMB_FX; ++i) {
        if (!g_bomb_fx[i].active) continue;
        double t = al_get_time() - g_bomb_fx[i].t0;
        if (t >= g_bomb_fx[i].dur) g_bomb_fx[i].active = false;
    }
}

// ───────── 그리기 ─────────
void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges) {
    // 그리드/타워
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(100, 100, 100), 1.0f);

            float cx = (x1 + x2) * 0.5f;
            float cy = (y1 + y2) * 0.5f;

            ALLEGRO_BITMAP* bmp = tower_anim_frame(&grid[r][c]);
            if (bmp) {
                float sw = (float)al_get_bitmap_width(bmp);
                float sh = (float)al_get_bitmap_height(bmp);
                float base = (CELL_W < CELL_H ? CELL_W : CELL_H) * 0.8f;
                float scale = base / ((sw > sh) ? sw : sh);
                float dw = sw * scale, dh = sh * scale;
                al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, cx - dw / 2, cy - dh / 2, dw, dh, 0);
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

    // 적
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy* e = &enemies[i];
        if (!e->active) continue;

        ALLEGRO_BITMAP* icon = NULL;
        switch (e->type) {
        case ET_FAST:   icon = icon_virus2; break;
        case ET_TANK:   icon = icon_virus1; break;
        case ET_BOMBER: icon = icon_virus3; break;
        case ET_FREEZER:icon = icon_virus4; break;
        default:        icon = icon_virus1; break;
        }

        if (icon) {
            float sw = (float)al_get_bitmap_width(icon);
            float sh = (float)al_get_bitmap_height(icon);
            float scale = 2.0f;
            float dw = sw * scale, dh = sh * scale;
            al_draw_scaled_bitmap(icon, 0, 0, sw, sh, e->x - dw * 0.5f, e->y - dh * 0.5f, dw, dh, 0);
        }
        else {
            al_draw_filled_rectangle(e->x - 8, e->y - 8, e->x + 8, e->y + 8, al_map_rgb(150, 50, 200));
        }

        // HP 바
        float er = (float)e->hp / (float)ENEMY_HP; if (er < 0) er = 0;
        float ex1 = e->x - 12, ey1 = e->y - 20, ex2 = e->x + 12, ey2 = ey1 + 4;
        al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
        al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * er, ey2, al_map_rgb(120, 220, 120));
    }

    // 공격 범위 표시
    if (show_ranges) {
        for (int r = 0; r < GRID_ROWS; ++r) for (int c = 0; c < GRID_COLS; ++c)
            if (grid[r][c].type == TOWER_ATTACK) {
                float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
                al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 120), 2.0f);
            }
    }
    else {
        if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS)
            if (grid[cursor_row][cursor_col].type == TOWER_ATTACK) {
                float x1, y1, x2, y2; cell_rect(cursor_row, cursor_col, &x1, &y1, &x2, &y2);
                float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
                al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 0, 150), 2.0f);
            }
    }

    // 커서
    if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
        float cx1, cy1, cx2, cy2; cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
        al_draw_rectangle(cx1 + 2, cy1 + 2, cx2 - 2, cy2 - 2, al_map_rgb(255, 255, 0), 3.0f);
    }

    // 총알
    for (int i = 0; i < MAX_BULLETS; ++i) if (bullets[i].active) {
        ALLEGRO_BITMAP* bullet_img = NULL;
    switch (bullets[i].image_type) { case 0: bullet_img = bullet_1; break; case 1: bullet_img = bullet_2; break; default: bullet_img = bullet_3; break; }
                                           if (bullet_img) {
                                               float iw = (float)al_get_bitmap_width(bullet_img);
                                               float ih = (float)al_get_bitmap_height(bullet_img);
                                               float scale = 3.0f;
                                               al_draw_scaled_bitmap(bullet_img, 0, 0, iw, ih,
                                                   bullets[i].x - (iw * 0.5f), bullets[i].y - (ih * 0.5f), iw * scale, ih * scale, 0);
                                           }
                                           else {
                                               al_draw_filled_circle(bullets[i].x, bullets[i].y, BULLET_RADIUS, al_map_rgb(255, 255, 255));
                                           }
    }

    // 폭발 이펙트(1초, 3프레임)
    for (int i = 0; i < MAX_BOMB_FX; ++i) {
        if (!g_bomb_fx[i].active) continue;
        double t = al_get_time() - g_bomb_fx[i].t0;
        double u = t / g_bomb_fx[i].dur;
        ALLEGRO_BITMAP* fx = NULL;
        if (u < 1.0 / 3.0) fx = icon_bombeffect1;
        else if (u < 2.0 / 3.0) fx = icon_bombeffect2;
        else                   fx = icon_bombeffect3;

        if (fx) {
            float sw = (float)al_get_bitmap_width(fx);
            float sh = (float)al_get_bitmap_height(fx);
            float scale = 3.0f;
            float dw = sw * scale, dh = sh * scale;
            al_draw_scaled_bitmap(fx, 0, 0, sw, sh, g_bomb_fx[i].x - dw * 0.5f, g_bomb_fx[i].y - dh * 0.5f, dw, dh, 0);
        }
    }
    // ───────── 스테이지 배너 표시 ─────────
    if (stage_banner_active) {
        double now = al_get_time();
        double total = 2.0;
        double time_left = stage_banner_until - now;
        if (time_left < 0) time_left = 0;
        double t = total - time_left;              // 0 → 2.0
        double a;                                   // alpha 0~1 (페이드 인/아웃)
        if (t < 0.25)        a = t / 0.25;          // 0~0.25s 페이드 인
        else if (t > 1.75)   a = (2.0 - t) / 0.25;  // 1.75~2.0s 페이드 아웃
        else                 a = 1.0;

        int alpha_bg = (int)(180 * a);
        int alpha_tx = (int)(255 * a);

        // 가운데 가로 바
        float bar_top = H * 0.40f;
        float bar_bot = H * 0.60f;
        al_draw_filled_rectangle(0, bar_top, (float)W, bar_bot,
            al_map_rgba(0, 0, 0, alpha_bg));

        // 텍스트
        ALLEGRO_FONT* f = font_title ? font_title : font_ui;
        al_draw_textf(f, al_map_rgba(255, 255, 255, alpha_tx),
            W / 2.0f, H * 0.445f, ALLEGRO_ALIGN_CENTER,
            "STAGE %d", stage_banner_stage);
    }
}

// ───────── 설치/판매/상태 ─────────
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
        grid[row][col].freeze_stacks = 0;
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
    t->freeze_stacks = 0;
}

GameState game_get_state(void) {
    return game_state;
}

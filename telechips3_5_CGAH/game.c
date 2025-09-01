// ================ game.c ================
#include "game.h"
#include "assets.h"
#include "sound.h"  // 사운드 시스템 추가
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
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

// ── 폭탄 사운드 관리 ──
static double bomb_sound_last_beep[MAX_ENEMIES];
static bool   bomb_sound_started[MAX_ENEMIES];

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

//냉동 바이러스에 의해 아군 유닛이 얼음 상태가 되면 아이콘 변경시키는 함수
static ALLEGRO_BITMAP* tower_anim_frame(const Tower* t) {
    if (t->freeze_stacks > 0) {
        switch (t->type) {
        case TOWER_ATTACK:   return icon_frozen1;
        case TOWER_RESOURCE: return icon_frozen2;
        case TOWER_TANK:     return icon_frozen3;
        default:             return icon_sleeping;
        }
    }
    // 평상시 아이콘. 에니메이션 효과 주기 위해 1초마다 아이콘 변경됨
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

static void clear_all_enemies(void) { //적 상태 초기화
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
        enemies[i].x = enemies[i].y = 0.0f;
        enemies[i].hp = 0;
        enemies[i].atk_cooldown = 0.0f;
        enemies[i].type = ET_FAST;
        enemies[i].speed = 0.0f;
        enemies[i].fuse_timer = 0.0f;
        enemies[i].freeze_link_count = 0;
        bomb_sound_last_beep[i] = 0.0;
        bomb_sound_started[i] = false;
    }
}
static void clear_all_bullets(void) {   //총알 초기화
    for (int i = 0; i < MAX_BULLETS; ++i) bullets[i].active = false;
}
static void clear_all_towers(void) { //아군 상태 초기화
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

static void begin_stage_banner(int stage) {         //스테이지 시작 시 안내 배너 출력
    stage_banner_stage = stage;
    stage_banner_active = true;
    stage_banner_until = al_get_time() + 2.0;
    last_enemy_spawn_time = stage_banner_until;
}

static void spawn_bomb_fx(float x, float y) {           //폭탄 스폰
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

// 오른쪽으로 직진하는 총알 발사 함수
static int spawn_bullet_row_right(float sx, float sy) {     // sx, sy총알을 생성할 시작 좌표(x, y)
    for (int i = 0; i < MAX_BULLETS; ++i) {                // MAX_BULLET 개수만큼 반복하면서, 아직 사용하지 않고 있는(active == false) 총알 슬롯을 찾음
        if (!bullets[i].active) {
            bullets[i].active = true;                      // 총알 활성화
            bullets[i].x = sx;
            bullets[i].y = sy;    // 생성 위치를 sx, sy로 지정
            bullets[i].vx = BULLET_SPEED; // 오른쪽으로 직진
            bullets[i].vy = 0.0f;         // 수평 유지(y축 변화 없음) 즉, 직선으로 오른쪽 이동
            bullets[i].image_type = rand() % 3;  // 총알의 외형을 3가지 중 랜덤하게 선택(* [] ->)
            return i;
        }
    }
    return -1;  // 반환값 : 총알이 저장된 배열 인덱스 (없으면 -1)
}

// ───────── 적 스폰 ─────────
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

            // 적 출현 타입 확률 지정하는 함수
            int r = rand() % 100;
            if (r < 35)       e->type = ET_TANK;
            else if (r < 70)  e->type = ET_FAST;
            else if (r < 85)  e->type = ET_BOMBER;
            else              e->type = ET_FREEZER;

            e->atk_cooldown = 0.0f;
            e->fuse_timer = 0.0f;
            e->freeze_link_count = 0;

            // 사운드 관련 초기화
            bomb_sound_last_beep[i] = 0.0;
            bomb_sound_started[i] = false;

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

// 스테이지별 적 스폰 규칙
static void get_stage_spawn_rule(int stage, float* interval_out, int* count_out) {
    float interval = 1.0f;
    int count = 1;

    if (stage <= 1) { interval = 3.0f; count = 1; }
    else if (stage == 2) { interval = 3.0f; count = 2; }
    else if (stage == 3) { interval = 2.0f; count = 1; }
    else if (stage == 4) { interval = 1.5f; count = 1; }
    else { interval = 1.0f; count = 1; }

    if (interval_out) *interval_out = interval;
    if (count_out) *count_out = count;
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

//자폭 바이러스의 폭발 효과 구현 함수
static void explode_bomb(Enemy* b) {
    sound_play(SOUND_VIRUS_EXPLODE);  // 폭발 사운드

    // 이펙트 표시
    spawn_bomb_fx(b->x, b->y);

    // 반경 내 타워의 HP를 절반으로 감소시킴
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

//냉동 바이러스의 얼림 효과 함수
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
        sound_play(SOUND_VIRUS4_FREEZE);  // 얼림 사운드
    }
}

// 원과 사각형 충돌 검사 함수
static bool circle_rect_overlap(float cx, float cy, float r, float rx1, float ry1, float rx2, float ry2) {  // 총알을 원, 적을 작은 사각형으로 보고 충돌 여부를 검사(원의 중심에서 사각형으로 클램핑한 최근접점까지의 제곱거리 <= r의 제곱이면 겹침
    float nnx = (cx < rx1) ? rx1 : (cx > rx2) ? rx2 : cx;
    float nny = (cy < ry1) ? ry1 : (cy > ry2) ? ry2 : cy;
    float dx = cx - nnx, dy = cy - nny;
    return (dx * dx + dy * dy) <= r * r;
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

    // 사운드 관련 초기화
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        bomb_sound_last_beep[i] = 0.0;
        bomb_sound_started[i] = false;
    }

    begin_stage_banner(1);
}

void game_reset(void) { //게임 진행상황 초기화
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

    // 타워 직진 발사 방식
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            Tower* t = &grid[r][c];
            float tcx = GRID_X + c * CELL_W + CELL_W * 0.5f;
            float tcy = GRID_Y + r * CELL_H + CELL_H * 0.5f;

            if (t->cooldown > 0.0f) t->cooldown -= dt;

            // 얼려 있으면 일절 동작 안 함
            if (t->freeze_stacks > 0) continue;

            if (t->type == TOWER_ATTACK && t->cooldown <= 0.0f) {
                // 직진 발사 로직: 같은 행 근처 + 타워의 오른쪽 + 사거리 내 적이 있으면 발사
                const float ROW_BAND = CELL_H * 0.45f;   // 같은 줄 판정 여유
                const float MIN_FIRE_DISTANCE = 24.0f;   // 너무 붙은 근접 발사 방지

                bool should_fire = false;
                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (!enemies[i].active) continue;

                    float dx = enemies[i].x - tcx;             // 오른쪽만
                    float dy = fabsf(enemies[i].y - tcy);      // 같은 행 대략 판정
                    float d = distancef(tcx, tcy, enemies[i].x, enemies[i].y);

                    if (dx > 0.0f && dy <= ROW_BAND && d <= ATTACK_TOWER_RANGE && d > MIN_FIRE_DISTANCE) {
                        // 오른쪽에 있고, 같은 줄이고, 사거리 안이며 너무 가깝지 않음
                        should_fire = true;
                        break;
                    }
                }

                if (should_fire) {
                    sound_play(SOUND_PEOPLE1_ATTACK);  // 공격 사운드
                    // 오른쪽 직진으로 총알 발사
                    spawn_bullet_row_right(tcx, tcy);
                    t->cooldown = ATTACK_TOWER_COOLDOWN;
                }
            }
            else if (t->type == TOWER_RESOURCE && t->cooldown <= 0.0f) {
                sound_play(SOUND_PEOPLE2_COIN);  // 자원 획득 사운드
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                t->cooldown = RESOURCE_TOWER_COOLDOWN;
            }
        }
    }

    // 적 로직
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy* e = &enemies[i];
        if (!e->active) continue;

        // 폭탄 타이머 및 사운드 처리
        if (e->type == ET_BOMBER && e->fuse_timer > 0.0f) {
            // 사운드 처리: 0.25초마다 타이머 소리 재생
            double now = al_get_time();
            if (!bomb_sound_started[i] || (now - bomb_sound_last_beep[i] >= 0.25)) {
                sound_play(SOUND_VIRUS3_TIMER);
                bomb_sound_last_beep[i] = now;
                bomb_sound_started[i] = true;
            }

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
            // 타입별 충돌 처리
            if (e->type == ET_BOMBER) {
                // 타워와 접촉하면 2초 뒤 자폭 (이미 카운트중이면 유지)
                if (e->fuse_timer <= 0.0f) {
                    e->fuse_timer = BOMB_ARM_TIME; // 2.0초로 변경됨
                    sound_play(SOUND_VIRUS3_TIMER);  // 첫 번째 타이머 사운드
                    bomb_sound_last_beep[i] = al_get_time();
                    bomb_sound_started[i] = true;
                }
            }
            else if (e->type == ET_FREEZER) {
                // 접촉한 타워 얼리기
                freezer_touch_tower(e, tgt_r, tgt_c);
            }

            // 적의 일반 공격(탱커/빠른 적 등)
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
                sound_play(SOUND_LIFE);  // 생명 감소 사운드
                game_state.lives--;
                if (game_state.lives <= 0) game_state.game_over = true;
            }
        }
    }
    // ── 적 스폰(스테이지별 규칙) ──
    {
        float spawn_interval = 0.0f;
        int spawn_count = 1;
        get_stage_spawn_rule(game_state.stage, &spawn_interval, &spawn_count);

        if (game_state.stage_kills < KILLS_TO_ADVANCE &&
            (al_get_time() - last_enemy_spawn_time) >= spawn_interval) {

            // interval마다 count마리씩 스폰
            for (int i = 0; i < spawn_count; ++i) {
                spawn_enemy();
            }
            last_enemy_spawn_time = al_get_time();
        }
    }
    // 총알 업데이트/피격 - 새로운 충돌 검사 방식 사용
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet* b = &bullets[i];
        if (!b->active) continue;

        b->x += b->vx * dt;
        b->y += b->vy * dt;

        float grid_right = GRID_X + GRID_COLS * CELL_W + 50;
        float grid_bottom = GRID_Y + GRID_ROWS * CELL_H + 50;
        if (b->x < -20 || b->x > grid_right || b->y < -20 || b->y > grid_bottom) {
            b->active = false;
            continue;
        }

        for (int eidx = 0; eidx < MAX_ENEMIES; ++eidx) {
            Enemy* e = &enemies[eidx];
            if (!e->active) continue;

            // 원과 사각형 충돌 검사 방식 사용
            float ex = e->x, ey = e->y;
            if (circle_rect_overlap(b->x, b->y, BULLET_RADIUS, ex - 8, ey - 8, ex + 8, ey + 8)) {
                b->active = false;

                e->hp -= ATTACK_TOWER_DAMAGE;
                if (e->hp <= 0) {
                    if (e->type == ET_BOMBER) {
                        // 총알로 죽어도 폭발은 발생
                        explode_bomb(e);
                    }
                    else {
                        e->active = false;
                        sound_play(SOUND_PEOPLE2_COIN);  // 처치 보상 사운드
                        game_state.caffeine += 20;
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

        // 폭탄 타이머 중이면 범위 표시
        if (e->type == ET_BOMBER && e->fuse_timer > 0.0f) {
            al_draw_circle(e->x, e->y, BOMB_RADIUS, al_map_rgba(255, 100, 100, 150), 3.0f);
        }
    }

    // 아군 공격 범위 표시
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
        switch (bullets[i].image_type) {
        case 0: bullet_img = bullet_1; break;
        case 1: bullet_img = bullet_2; break;
        default: bullet_img = bullet_3; break;
        }
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

// ───────── 설치/상태 ─────────
void game_place_tower(TowerType type, int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    if (grid[row][col].type != TOWER_EMPTY) return;

    int cost = 0;
    if (type == TOWER_ATTACK)        cost = ATTACK_TOWER_COST;
    else if (type == TOWER_RESOURCE) cost = RESOURCE_TOWER_COST;
    else if (type == TOWER_TANK)     cost = TANK_TOWER_COST;

    if (game_state.caffeine >= cost) {
        sound_play(SOUND_BUY);  // 타워 구매 사운드
        game_state.caffeine -= cost;
        grid[row][col].type = type;
        grid[row][col].cooldown = 0.0f;
        grid[row][col].hp = tower_max_hp(type);
        grid[row][col].freeze_stacks = 0;
    }
}

GameState game_get_state(void) {
    return game_state;
}
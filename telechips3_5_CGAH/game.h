#pragma once
#include <stdbool.h>
#include "app.h"  // GRID_ROWS, GRID_COLS

#define STARTING_CAFFEINE 200
#define MAX_STAGES 5
#define KILLS_TO_ADVANCE 10
#define MAX_ENEMIES 50
#define ENEMY_HP 100
#define ENEMY_SPEED 0.8f

#define ATTACK_TOWER_COST 100
#define RESOURCE_TOWER_COST 75
#define TANK_TOWER_COST 50

#define ATTACK_TOWER_RANGE 100.0f
#define ATTACK_TOWER_DAMAGE 25
#define ATTACK_TOWER_COOLDOWN 0.8f

#define RESOURCE_TOWER_AMOUNT 15
#define RESOURCE_TOWER_COOLDOWN 1.0f

#define ATTACK_TOWER_HP   200
#define RESOURCE_TOWER_HP 150
#define TANK_TOWER_HP     400

#define ENEMY_ATTACK_RANGE   18.0f
#define ENEMY_ATTACK_DAMAGE  15
#define ENEMY_ATTACK_COOLDOWN 0.8f

// 발사체 (총알)
#define MAX_BULLETS   128
#define BULLET_SPEED  120.0f
#define BULLET_RADIUS 20.0f

typedef enum { TOWER_EMPTY, TOWER_ATTACK, TOWER_RESOURCE, TOWER_TANK } TowerType;

typedef enum {
    ET_FAST = 0,
    ET_TANK,
    ET_BOMBER,
    ET_FREEZER
} EnemyType;

// 적 유닛별 스탯
#define FAST_HP     60
#define FAST_SPEED  100.0f

#define TANK_HP     180
#define TANK_SPEED  20.0f

#define BOMB_HP               50
#define BOMB_SPEED            28.0f
#define BOMB_ARM_TIME         2.0f    // ★ 타워와 충돌한 뒤 자폭까지 대기 시간
#define BOMB_RADIUS           60.0f
#define BOMB_DAMAGE           140     // (현재는 미사용, 필요시 반경내 추가피해 등으로 사용 가능)

// FREEZER
#define FREEZER_HP            80
#define FREEZER_SPEED         24.0f
#define FREEZER_MAX_LINKS     32      // FREEZER 한 마리당 얼릴 수 있는 타워 최대개수

typedef struct {
    TowerType type;
    float cooldown;
    int   hp;
    float stun_timer;     // (미사용)
    int   freeze_stacks;  // ★ 0보다 크면 동작 정지(얼림). 아이콘도 얼림 아이콘으로 교체
} Tower;

typedef struct {
    bool active;
    float x, y;
    int   hp;
    float atk_cooldown;
    EnemyType type;
    float speed;

    // ★ BOMBER 전용: 타워와 붙은 뒤 자폭 타이머(>0이면 카운트중)
    float fuse_timer;

    // ★ FREEZER 전용: 얼려둔 타워 좌표 목록(죽을 때 해제)
    int   freeze_link_count;
    struct { int r, c; } freeze_links[FREEZER_MAX_LINKS];
} Enemy;

typedef struct {
    int caffeine;
    int lives;
    int stage;
    int stage_kills;
    bool cleared;
    bool game_over;
} GameState;

typedef struct {
    bool  active;
    float x, y;
    float vx, vy;
    int   image_type;
} Bullet;

void game_init(void);
void game_update(float dt);
void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges);
void game_place_tower(TowerType type, int row, int col);
void game_sell_tower(int row, int col);
GameState game_get_state(void);
void game_reset(void);

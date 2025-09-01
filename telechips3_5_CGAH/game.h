#pragma once
#include <stdbool.h>
#include "app.h"  // GRID_ROWS, GRID_COLS를 위해 app.h 포함

// 진행 기본값
#define STARTING_CAFFEINE 300
#define MAX_STAGES 5
#define KILLS_TO_ADVANCE 15

#define MAX_ENEMIES 50
#define ENEMY_HP 100
#define ENEMY_SPEED 0.8f

#define ATTACK_TOWER_COST 100
#define RESOURCE_TOWER_COST 75
#define TANK_TOWER_COST 50
#define ATTACK_TOWER_RANGE 300.0f // 공격 유닛 사거리
#define ATTACK_TOWER_DAMAGE 12
#define ATTACK_TOWER_COOLDOWN 0.25f // 공격 유닛 쿨타임
#define RESOURCE_TOWER_AMOUNT 15
#define RESOURCE_TOWER_COOLDOWN 2.5f
#define ATTACK_TOWER_HP 200
#define RESOURCE_TOWER_HP 150
#define TANK_TOWER_HP 400
#define ENEMY_ATTACK_RANGE 18.0f
#define ENEMY_ATTACK_DAMAGE 15
#define ENEMY_ATTACK_COOLDOWN 0.8f

// 발사체 (총알)
#define MAX_BULLETS   128      // 동시에 존재 가능한 총알(발사체) 슬롯 수
#define BULLET_SPEED  120.0f   // 총알이 1초에 120픽셀 이동
#define BULLET_RADIUS 20.0f    // 충돌 판정용 반지름(그림 크기 아님), 총알 크기 바꾸고 싶으면 -> BULLET_RADIUS값 바꾸기 / 화면에 보이는 크기 : game_draw_grid() 안 총알 그리기에서 float scale = 3.0f; 조정

typedef enum { TOWER_EMPTY, TOWER_ATTACK, TOWER_RESOURCE, TOWER_TANK } TowerType;

typedef enum {
    ET_FAST = 0,
    ET_TANK,
    ET_BOMBER,
    ET_FREEZER
} EnemyType;

// 적 유닛별 스탯 정의
#define FAST_HP     60
#define FAST_SPEED  70.0f
#define TANK_HP     180
#define TANK_SPEED  20.0f
#define BOMB_HP               50
#define BOMB_SPEED            28.0f
#define BOMB_FUSE_TIME        0.75f
#define BOMB_RADIUS          100.0f
#define BOMB_DAMAGE           140
#define BOMB_ARM_TIME 2.0f    // 폭탄 타이머 (2초)
#define FREEZER_HP            150
#define FREEZER_SPEED         24.0f
#define FREEZER_STUN_DUR      2.5f   // (미사용, 참고용)
#define FREEZER_COOLDOWN      3.0f   // (미사용, 참고용)

// ★ FREEZER가 한 적 당 얼릴 수 있는 타워 최대 개수
#define FREEZER_MAX_LINKS     32

typedef struct { 
    TowerType type;
    float cooldown;
    int hp;
    float stun_timer;     // (기존 필드, 현재 미사용)
    int  freeze_stacks;   // ★ FREEZER에게 얼려진 횟수(>0이면 동작 멈춤)
} Tower;

typedef struct { //적 공격유닛 구성요소 구조체
    bool active;
    float x, y;
    int hp;
    float atk_cooldown;
    EnemyType type;
    float speed;
    float fuse_timer;

    // ★ FREEZER 전용: 자신이 얼려둔 타워 목록(해제 위해 추적)
    int  freeze_link_count;
    struct { int r, c; } freeze_links[FREEZER_MAX_LINKS];
} Enemy;

typedef struct { //게임 진행요소 구조체
    int caffeine;
    int lives;
    int stage;
    int stage_kills;
    bool cleared;
    bool game_over;
} GameState;

typedef struct { //총알 오브젝트 구조체
    bool  active;
    float x, y;     // 위치(px)
    float vx, vy;   // 속도(px/s)
    int   image_type;
} Bullet;

// 스테이지별 적군 스탯 테이블 정의
typedef struct {
    float speed_multiplier;
    int damage_bonus;  // 기본 공격력에 더할 추가 데미지
} StageEnemyStats;

void game_init(void);
void game_update(float dt);
void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges);
void game_place_tower(TowerType type, int row, int col);
GameState game_get_state(void);
void game_reset(void);
StageEnemyStats get_stage_enemy_stats(int stage);


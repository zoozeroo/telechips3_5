#include "game.h"
#include "assets.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>   // ���/����Ʈ ��¿�

// ������������������ �׸��� ���̾ƿ� ������������������
#define CELL_W 102
#define CELL_H 102
#define GRID_X 21
#define GRID_Y 186

static Tower  grid[GRID_ROWS][GRID_COLS];
static Enemy  enemies[MAX_ENEMIES];
static Bullet bullets[MAX_BULLETS];
static GameState game_state;

static double last_enemy_spawn_time = 0.0;

// ���� �������� ��� ���� (2�� ���� ���� �Ͻ� ����) ����
static bool   stage_banner_active = false;
static double stage_banner_until = 0.0;
static int    stage_banner_stage = 0;

// ���� BOMB ���� ����Ʈ ����
#define MAX_BOMB_FX 32
typedef struct {
    bool   active;
    float  x, y;
    double start_time;
} BombFX;
static BombFX g_bombfx[MAX_BOMB_FX];

// ������������������ ��ƿ ������������������
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

static ALLEGRO_BITMAP* tower_anim_frame(const Tower* cell) {
    const TowerType t = cell ? cell->type : TOWER_EMPTY;

    // ����ִ� ĭ
    if (t == TOWER_EMPTY) return icon_sleeping;

    // �� ����ִ� ������ Ÿ�Ժ� frozen ���������� ���� ǥ��
    if (cell && cell->freeze_stacks > 0) {
        switch (t) {
        case TOWER_ATTACK:   return icon_frozen1;
        case TOWER_RESOURCE: return icon_frozen2;
        case TOWER_TANK:     return icon_frozen3; // ��Ŀ�� ��� ��ȭ�� ������ �ð������θ� �� ǥ��
        default:             return icon_sleeping;
        }
    }

    // ���� �ִϸ��̼�
    bool blink = anim_blink_1s();
    switch (t) {
    case TOWER_ATTACK:
        return blink ? (icon_people1 ? icon_people1 : icon_sleeping)
            : (icon_people1_1 ? icon_people1_1 : icon_people1);
    case TOWER_RESOURCE:
        return blink ? (icon_people2 ? icon_people2 : icon_sleeping)
            : (icon_people2_1 ? icon_people2_1 : icon_people2);
    case TOWER_TANK:
        return blink ? (icon_people3 ? icon_people3 : icon_sleeping)
            : (icon_people3_1 ? icon_people3_1 : icon_people3);
    default:
        return icon_sleeping;
    }
}

// ������������������ �ʱ�ȭ ���� ������������������
static void clear_all_enemies(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].active = false;
        enemies[i].x = 0.0f;
        enemies[i].y = 0.0f;
        enemies[i].hp = 0;
        enemies[i].atk_cooldown = 0.0f;
        enemies[i].type = ET_FAST;      // �⺻��
        enemies[i].speed = 0.0f;
        enemies[i].fuse_timer = 0.0f;
        enemies[i].freeze_link_count = 0; // �� FREEZER ��ũ �ʱ�ȭ
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
            grid[r][c].freeze_stacks = 0; // �� �� ����
        }
    }
}
static void clear_all_bombfx(void) {
    for (int i = 0; i < MAX_BOMB_FX; ++i) g_bombfx[i].active = false;
}
static void begin_stage_banner(int stage) {
    stage_banner_stage = stage;
    stage_banner_active = true;
    stage_banner_until = al_get_time() + 2.0;    // �� 2��
    last_enemy_spawn_time = stage_banner_until;   // ��� ���� ���ĺ��� ���� Ÿ�̹� ī��Ʈ
}

// ������������������ ��/�Ѿ� ���� ������������������
static void spawn_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) {
            Enemy* e = &enemies[i];
            e->active = true;

            // ������ �������ڸ� ����
            float right_edge = GRID_X + GRID_COLS * CELL_W;
            e->x = right_edge + 20.0f;

            // ���� �� �߾ӿ� ��Ȯ�� ��ġ
            int row = rand() % GRID_ROWS;
            float x1, y1, x2, y2;
            cell_rect(row, 0, &x1, &y1, &x2, &y2);
            e->y = 0.5f * (y1 + y2);

            // Ÿ�� Ȯ��(����ġ) ����
            int r = rand() % 100;
            if (r < 50)      e->type = ET_FAST;     // 50%
            else if (r < 85) e->type = ET_TANK;     // 35%
            else if (r < 95) e->type = ET_BOMBER;   // 10%
            else             e->type = ET_FREEZER;  // 5%

            // Ÿ�Ժ� ���� ����
            e->atk_cooldown = 0.0f;
            e->fuse_timer = 0.0f;
            e->freeze_link_count = 0; // �� FREEZER ��ũ �ʱ�ȭ
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

// �� ���������� ���� ��Ģ: (interval �ʸ��� count����)
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

// �� ��ź �� ��� �� ���� ȿ��: �ݰ� �� Ÿ�� HP �������� ����
static void bomber_explode(float ex, float ey) {
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            Tower* t = &grid[r][c];
            if (t->type == TOWER_EMPTY) continue;

            float x1, y1, x2, y2;
            cell_rect(r, c, &x1, &y1, &x2, &y2);
            float cx = (x1 + x2) * 0.5f;
            float cy = (y1 + y2) * 0.5f;

            if (distancef(ex, ey, cx, cy) <= BOMB_RADIUS) {
                t->hp = t->hp / 2;           // ���ݸ� ����
                if (t->hp <= 0) {
                    t->type = TOWER_EMPTY;
                    t->hp = 0;
                    t->cooldown = 0.0f;
                    t->freeze_stacks = 0;    // Ȥ�� ����� �ִ� �� �ʱ�ȭ
                }
            }
        }
    }
}

// �� ���� ����Ʈ ����/������Ʈ/�׸���
static void spawn_bomb_fx(float x, float y) {
    for (int i = 0; i < MAX_BOMB_FX; ++i) {
        if (!g_bombfx[i].active) {
            g_bombfx[i].active = true;
            g_bombfx[i].x = x;
            g_bombfx[i].y = y;
            g_bombfx[i].start_time = al_get_time();
            return;
        }
    }
}
static void update_bomb_fx(void) {
    double now = al_get_time();
    for (int i = 0; i < MAX_BOMB_FX; ++i) {
        if (!g_bombfx[i].active) continue;
        if (now - g_bombfx[i].start_time >= 1.0) { // 1�� ����
            g_bombfx[i].active = false;
        }
    }
}
static void draw_bomb_fx(void) {
    double now = al_get_time();
    for (int i = 0; i < MAX_BOMB_FX; ++i) {
        if (!g_bombfx[i].active) continue;

        double t = now - g_bombfx[i].start_time; // [0,1]
        ALLEGRO_BITMAP* fx = NULL;
        if (t < 1.0 / 3.0)       fx = icon_bombeffect1;
        else if (t < 2.0 / 3.0)  fx = icon_bombeffect2;
        else                   fx = icon_bombeffect3;

        if (fx) {
            float sw = (float)al_get_bitmap_width(fx);
            float sh = (float)al_get_bitmap_height(fx);
            float scale = 2.5f; // ����Ʈ ũ��
            float dw = sw * scale, dh = sh * scale;
            al_draw_scaled_bitmap(
                fx, 0, 0, sw, sh,
                g_bombfx[i].x - dw * 0.5f,
                g_bombfx[i].y - dh * 0.5f,
                dw, dh, 0
            );
        }
        else {
            al_draw_filled_circle(g_bombfx[i].x, g_bombfx[i].y, 30.0f, al_map_rgb(255, 255, 255));
        }
    }
}

// ������������������ FREEZER ��ƿ ������������������
static bool freezer_has_link(Enemy* e, int r, int c) {
    for (int i = 0; i < e->freeze_link_count; ++i) {
        if (e->freeze_links[i].r == r && e->freeze_links[i].c == c) return true;
    }
    return false;
}
static void freezer_add_link(Enemy* e, int r, int c) {
    if (e->freeze_link_count >= FREEZER_MAX_LINKS) return;
    if (freezer_has_link(e, r, c)) return;

    int idx = e->freeze_link_count;
    e->freeze_links[idx].r = r;
    e->freeze_links[idx].c = c;
    e->freeze_link_count = idx + 1;

    grid[r][c].freeze_stacks += 1;
}
static void freezer_clear_links(Enemy* e) {
    for (int i = 0; i < e->freeze_link_count; ++i) {
        int r = e->freeze_links[i].r;
        int c = e->freeze_links[i].c;
        if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS) {
            if (grid[r][c].freeze_stacks > 0) grid[r][c].freeze_stacks -= 1;
        }
    }
    e->freeze_link_count = 0;
}

// ������������������ �ݹ�: �� óġ ������������������
static void on_enemy_killed(void) {
    game_state.stage_kills++;
    if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
        // ������ ���������� Ŭ����
        if (game_state.stage >= MAX_STAGES) {
            game_state.cleared = true;
            clear_all_enemies();
            clear_all_bullets();
            clear_all_bombfx();
            return;
        }

        // ���� �������� �غ�: ��/�Ѿ�/Ÿ�� ����, ��ȭ �ʱ�ȭ, ��� 2��
        clear_all_enemies();
        clear_all_bullets();
        clear_all_towers();
        clear_all_bombfx();

        game_state.stage++;
        game_state.stage_kills = 0;
        game_state.caffeine = STARTING_CAFFEINE;   // �� ��ȭ �ʱ�ȭ

        begin_stage_banner(game_state.stage);
    }
}

// ������������������ �ۺ� API ������������������
void game_init(void) {
    srand((unsigned)time(NULL));

    clear_all_towers();
    clear_all_enemies();
    clear_all_bullets();
    clear_all_bombfx();

    game_state.caffeine = STARTING_CAFFEINE; // �� ���� ��ȭ
    game_state.lives = 5;
    game_state.stage = 1;
    game_state.stage_kills = 0;
    game_state.cleared = false;
    game_state.game_over = false;

    begin_stage_banner(1); // �� �������� 1 ��� 2��
}

void game_reset(void) {
    clear_all_enemies();
    clear_all_bullets();
    clear_all_bombfx();
    game_state.game_over = false;
    game_state.cleared = false;
}

// ���� ������Ʈ
void game_update(float dt) {
    if (game_state.cleared || game_state.game_over) return;

    // ���� ��� �߿� ���� ���� ����
    if (stage_banner_active) {
        if (al_get_time() < stage_banner_until) return;
        stage_banner_active = false;               // ��� ���� �� �׶����� ���� ����
        last_enemy_spawn_time = al_get_time();
    }

    // ���� Ÿ�� ���� ����
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            bool is_attack = (grid[r][c].type == TOWER_ATTACK);
            bool is_resource = (grid[r][c].type == TOWER_RESOURCE);
            bool frozen = (grid[r][c].freeze_stacks > 0) && (is_attack || is_resource);

            // ��ٿ� ����
            if (grid[r][c].cooldown > 0.0f) {
                if (!frozen) grid[r][c].cooldown -= dt;  // ��������� ��ٿ ����
            }

            if (frozen) continue; // ����� ������ ��� ����

            float tcx = GRID_X + c * CELL_W + CELL_W * 0.5f;
            float tcy = GRID_Y + r * CELL_H + CELL_H * 0.5f;

            if (is_attack && grid[r][c].cooldown <= 0.0f) {
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
            else if (is_resource && grid[r][c].cooldown <= 0.0f) {
                game_state.caffeine += RESOURCE_TOWER_AMOUNT;
                grid[r][c].cooldown = RESOURCE_TOWER_COOLDOWN;
            }
        }
    }

    // ���� �� ���� ����
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        if (enemies[i].atk_cooldown > 0.0f) enemies[i].atk_cooldown -= dt;

        // FREEZER: �浹 �� Ÿ�� �� �ο�(����)
        if (enemies[i].type == ET_FREEZER) {
            for (int r = 0; r < GRID_ROWS; ++r) {
                for (int c = 0; c < GRID_COLS; ++c) {
                    Tower* t = &grid[r][c];
                    if (!(t->type == TOWER_ATTACK || t->type == TOWER_RESOURCE)) continue; // ��Ŀ/��ĭ ����

                    float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
                    float cx = (x1 + x2) * 0.5f, cy = (y1 + y2) * 0.5f;
                    float d = distancef(enemies[i].x, enemies[i].y, cx, cy);

                    // "�浹" ����: ���� �� ���� ��Ÿ� ���
                    if (d <= ENEMY_ATTACK_RANGE) {
                        freezer_add_link(&enemies[i], r, c);
                    }
                }
            }
        }

        // �Ϲ� Ÿ�� Ÿ���� �� ����
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
                    target_cell->freeze_stacks = 0; // �ı� �� �󸲵� ����
                }
            }
        }
        else {
            enemies[i].x -= enemies[i].speed * dt;
            if (enemies[i].x <= GRID_X - 20) {
                // �� ȭ�� ������ ���� ��Ȱ��ȭ�Ǳ� �� FREEZER �� ����
                if (enemies[i].type == ET_FREEZER) freezer_clear_links(&enemies[i]);
                enemies[i].active = false;
                game_state.lives--;
                if (game_state.lives <= 0) game_state.game_over = true;
            }
        }
    }

    // ���� �� ����(���������� ��Ģ) ����
    {
        float spawn_interval = 0.0f;
        int spawn_count = 1;
        get_stage_spawn_rule(game_state.stage, &spawn_interval, &spawn_count);

        if (game_state.stage_kills < KILLS_TO_ADVANCE &&
            (al_get_time() - last_enemy_spawn_time) >= spawn_interval) {

            // interval���� count������ ����
            for (int i = 0; i < spawn_count; ++i) {
                spawn_enemy();
            }
            last_enemy_spawn_time = al_get_time();
        }
    }

    // ���� �Ѿ� ������Ʈ ����
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
                    // �� BOMB ����
                    if (enemies[e].type == ET_BOMBER) {
                        bomber_explode(ex, ey);
                        spawn_bomb_fx(ex, ey);
                    }
                    // �� FREEZER ����
                    if (enemies[e].type == ET_FREEZER) {
                        freezer_clear_links(&enemies[e]);
                    }

                    enemies[e].active = false;
                    game_state.caffeine += 20;
                    on_enemy_killed();
                }

                b->active = false;
                break;
            }
        }
    }

    // ���� ���� ����Ʈ ������Ʈ ����
    update_bomb_fx();
}

// ���� �׸���
void game_draw_grid(int W, int H, int cursor_col, int cursor_row, bool show_ranges) {
    // ���� �׸��� / Ÿ�� ����
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(100, 100, 100), 1.0f);

            float cx = (x1 + x2) * 0.5f;
            float cy = (y1 + y2) * 0.5f;

            ALLEGRO_BITMAP* tower_sprite = tower_anim_frame(&grid[r][c]);
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

    // ���� �� ����
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].active) continue;

        // Ÿ�Ժ� �̹��� ����
        ALLEGRO_BITMAP* icon = NULL;
        switch (enemies[i].type) {
        case ET_FAST:   icon = icon_virus2; break;  // ���� �� (�����)
        case ET_TANK:   icon = icon_virus1; break;  // ��ũ �� (�Ķ���)
        case ET_BOMBER: icon = icon_virus3; break;  // ��ź �� (������)
        case ET_FREEZER:icon = icon_virus4; break;  // �󸮴� �� (�ϴû�)
        default:        icon = icon_virus1; break;
        }

        if (icon) {
            float sw = (float)al_get_bitmap_width(icon);
            float sh = (float)al_get_bitmap_height(icon);
            float scale = 2.0f;
            float dw = sw * scale, dh = sh * scale;
            al_draw_scaled_bitmap(icon, 0, 0, sw, sh,
                enemies[i].x - dw * 0.5f,
                enemies[i].y - dh * 0.5f,
                dw, dh, 0);
        }
        else {
            // ����: Ÿ�Ժ� ���� �簢��
            ALLEGRO_COLOR col;
            switch (enemies[i].type) {
            case ET_FAST:   col = al_map_rgb(255, 220, 70); break;   // ���
            case ET_TANK:   col = al_map_rgb(70, 120, 255); break;   // �Ķ�
            case ET_BOMBER: col = al_map_rgb(255, 100, 100); break;  // ����
            case ET_FREEZER:col = al_map_rgb(120, 200, 255); break;  // �ϴ�
            default:        col = al_map_rgb(150, 50, 200); break;
            }
            al_draw_filled_rectangle(enemies[i].x - 8, enemies[i].y - 8,
                enemies[i].x + 8, enemies[i].y + 8, col);
        }

        float er = (float)enemies[i].hp / (float)ENEMY_HP; if (er < 0) er = 0;
        float ex1 = enemies[i].x - 12, ey1 = enemies[i].y - 20, ex2 = enemies[i].x + 12, ey2 = ey1 + 4;
        al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
        al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * er, ey2, al_map_rgb(120, 220, 120));
    }

    // ���� ���� ����Ʈ �׸��� (��/Ÿ�� ����) ����
    draw_bomb_fx();

    // ���� ���� ���� ����
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

    // ���� Ŀ�� ����
    if (cursor_row >= 0 && cursor_row < GRID_ROWS && cursor_col >= 0 && cursor_col < GRID_COLS) {
        float cx1, cy1, cx2, cy2; cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
        al_draw_rectangle(cx1 + 2, cy1 + 2, cx2 - 2, cy2 - 2, al_map_rgb(255, 255, 0), 3.0f);
    }

    // ���� �Ѿ� ����
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].active) continue;
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
                bullets[i].x - (iw * 0.5f),
                bullets[i].y - (ih * 0.5f),
                iw * scale, ih * scale, 0);
        }
        else {
            al_draw_filled_circle(bullets[i].x, bullets[i].y, BULLET_RADIUS, al_map_rgb(255, 255, 255));
        }
    }

    // ���� STAGE ���(2��) ����
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

// ���� ��ġ / �Ǹ� ����
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
        grid[row][col].freeze_stacks = 0; // �� Ÿ���� ��� ����
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

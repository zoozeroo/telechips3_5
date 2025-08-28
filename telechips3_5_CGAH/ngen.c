// Allegro 5 C 예제 — 타워 디펜스 기능 확장
// 공격/자원 타워, 적 유닛, 게임 상태 추가
// 수정: 적이 오른쪽에서 무작위로 등장하여 왼쪽으로 이동하도록 변경

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h> // rand(), srand() 사용
#include <time.h>   // time() 사용

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
//#include <allegro5/allegro_ttf.h>


#define BUFFER_W 320
#define BUFFER_H 240

#define DISP_SCALE 3
#define DISP_W (BUFFER_W * DISP_SCALE)
#define DISP_H (BUFFER_H * DISP_SCALE)

#define COLS 7
#define ROWS 5
#define CELL_W 34
#define CELL_H 33.5
#define GRID_X 7
#define GRID_Y 64

//스테이지 기준
#define MAX_STAGES 5
#define KILLS_TO_ADVANCE 10

#define MAX_ENEMIES 50
#define ENEMY_HP 100
#define ENEMY_SPEED 0.1f // 적 이동 속도

// 타워 가격
#define ATTACK_TOWER_COST 100
#define RESOURCE_TOWER_COST 75
#define TANK_TOWER_COST 50

// 타워 특성
#define ATTACK_TOWER_RANGE 50.0f // 타워 공격 범위
#define ATTACK_TOWER_DAMAGE 5   // 타워 공격력
#define ATTACK_TOWER_COOLDOWN 0.1f // 공격 쿨다운 (초)
#define RESOURCE_TOWER_AMOUNT 5
#define RESOURCE_TOWER_COOLDOWN 1.0f // 자원 생성 쿨다운 (초)
#define ATTACK_TOWER_HP        200 // ===== Tower HP =====
#define RESOURCE_TOWER_HP      150
#define TANK_TOWER_HP		   400

// ===== Enemy melee attack params =====
#define ENEMY_ATTACK_RANGE     18.0f   // 근접 사정거리(픽셀)
#define ENEMY_ATTACK_DAMAGE    10      // 타워에 주는 피해
#define ENEMY_ATTACK_COOLDOWN  0.5f    // 공격 쿨다운(초)

// 타워 종류
typedef enum {
	TOWER_EMPTY,
	TOWER_ATTACK,
	TOWER_RESOURCE,
	TOWER_TANK
} TowerType;

// 타워 구조체
typedef struct {
	TowerType type;
	float cooldown;
	int hp;// 공격 또는 자원 생산 쿨다운 타이머,체력
} Tower;

// 적 구조체
typedef struct {
	bool active;
	float x, y;
	int hp;
	float atk_cooldown; // 근접공격 쿨다운
} Enemy;


// 게임 상태 구조체
typedef struct {
	int caffeine;
	int lives;
	int stage;        // 현재 스테이지 (1부터 시작)
	int stage_kills;  // 이번 스테이지에서 잡은 수
	bool cleared;     // 모든 스테이지 클리어 여부
} GameState;

// 전역 변수
static Tower grid[ROWS][COLS];
static Enemy enemies[MAX_ENEMIES];
static GameState game_state;
static int cursor_row = 0;
static int cursor_col = 0;
static bool show_all_ranges = false; // R 키로 토글
static double last_enemy_spawn_time = 0.0;

// Allegro 객체
ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;
ALLEGRO_FONT* game_font;

// --- 함수 선언 ---
void init_game_objects();
void update_game(float dt);
void draw_game(ALLEGRO_BITMAP* background);
void place_tower(TowerType type);
void sell_tower();
void spawn_enemy();
void reset_all_enemies(void);
void on_enemy_killed(void);
void advance_stage(void);

//타워 최대 HP
static inline int tower_max_hp(TowerType t) {
	if (t == TOWER_ATTACK)  return ATTACK_TOWER_HP;
	if (t == TOWER_RESOURCE) return RESOURCE_TOWER_HP;
	if (t == TOWER_TANK) return TANK_TOWER_HP;
	return 0;
}

// 셀 좌표를 픽셀 사각형 좌표로 변환
static void cell_rect(int row, int col, float* x1, float* y1, float* x2, float* y2) {
	*x1 = GRID_X + col * CELL_W;
	*y1 = GRID_Y + row * CELL_H;
	*x2 = *x1 + CELL_W;
	*y2 = *y1 + CELL_H;
}

// 두 점 사이의 거리를 계산 (피타고라스 정리)
float distance(float x1, float y1, float x2, float y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// 게임 객체 초기화
void init_game_objects() {
	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
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

	last_enemy_spawn_time = al_get_time();
}

void reset_all_enemies(void) {
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		enemies[i].active = false;
	}
}

void advance_stage(void) {
	// 10마리 달성 시 호출됨
	reset_all_enemies();

	if (game_state.stage >= MAX_STAGES) {
		// 모든 스테이지 클리어
		game_state.cleared = true;
		return;
	}

	game_state.stage++;
	game_state.stage_kills = 0;

	// 다음 스테이지 준비: 스폰 타이밍 리셋
	last_enemy_spawn_time = al_get_time();

	// (선택) 난이도 조절을 여기에서 가능:
	// 예: ENEMY_SPEED를 런타임 변수로 빼서 조금 올리기, 스폰 간격 줄이기 등
}

// 새로운 적 생성
void spawn_enemy() {
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		if (!enemies[i].active) {
			enemies[i].active = true;
			// 화면 오른쪽(BUFFER_W)에서 등장
			enemies[i].x = BUFFER_W;
			// Y좌표는 그리드 영역 내에서 무작위로 설정
			enemies[i].y = GRID_Y + (rand() % (int)(ROWS * CELL_H));
			enemies[i].hp = ENEMY_HP;
			enemies[i].atk_cooldown = 0.0f;
			break; // 한 마리만 생성하고 종료
		}
	}
}
void on_enemy_killed(void) {
	game_state.stage_kills++;
	if (game_state.stage_kills >= KILLS_TO_ADVANCE) {
		advance_stage();
	}
}

// 게임 로직 업데이트 (dt: delta time, 프레임 간 시간 간격)
void update_game(float dt) {
	// --- 타워 로직 업데이트 ---
	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			// 타워의 중심 좌표 계산
			float tower_cx = GRID_X + c * CELL_W + CELL_W / 2.0f;
			float tower_cy = GRID_Y + r * CELL_H + CELL_H / 2.0f;

			// 쿨다운 감소
			if (grid[r][c].cooldown > 0) {
				grid[r][c].cooldown -= dt;
			}

			// 공격 타워 로직
			if (grid[r][c].type == TOWER_ATTACK && grid[r][c].cooldown <= 0) {
				Enemy* target = NULL;
				float min_dist = ATTACK_TOWER_RANGE; // 최대 사정거리를 초기 최소 거리로 설정

				// 가장 가까운 적 찾기
				for (int i = 0; i < MAX_ENEMIES; ++i) {
					if (enemies[i].active) {
						// 타워와 적 사이의 거리 계산
						float d = distance(tower_cx, tower_cy, enemies[i].x, enemies[i].y);
						// 거리가 사정거리 내에 있고, 현재까지 발견한 가장 가까운 적보다 더 가깝다면
						if (d < min_dist) {
							min_dist = d; // 최소 거리 갱신
							target = &enemies[i]; // 목표물로 설정
						}
					}
				}

				// 목표물을 찾았다면 공격
				if (target) {
					target->hp -= ATTACK_TOWER_DAMAGE; // 적 체력 감소
					// 적의 체력이 0 이하면 비활성화
					if (target->hp <= 0) {
						target->active = false;
						game_state.caffeine += 20; // 적 제거 보상
						on_enemy_killed(); //킬 집계
					}
					grid[r][c].cooldown = ATTACK_TOWER_COOLDOWN; // 공격 쿨다운 초기화
				}
			}
			// 자원 타워 로직
			else if (grid[r][c].type == TOWER_RESOURCE && grid[r][c].cooldown <= 0) {
				game_state.caffeine += RESOURCE_TOWER_AMOUNT;
				grid[r][c].cooldown = RESOURCE_TOWER_COOLDOWN;
			}
		}
	}

	// --- 적 로직 업데이트 ---
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		if (!enemies[i].active) continue;

		// 쿨다운 감소
		if (enemies[i].atk_cooldown > 0.0f)
			enemies[i].atk_cooldown -= dt;

		// 가장 가까운 타워 탐색 (근접 사정거리 내)
		Tower* target_cell = NULL;
		int target_r = -1, target_c = -1;
		float best_d = ENEMY_ATTACK_RANGE;

		for (int r = 0; r < ROWS; ++r) {
			for (int c = 0; c < COLS; ++c) {
				if (grid[r][c].type == TOWER_EMPTY) continue;
				// 타워 중심
				float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
				float tcx = (x1 + x2) * 0.5f;
				float tcy = (y1 + y2) * 0.5f;
				float d = distance(enemies[i].x, enemies[i].y, tcx, tcy);
				if (d < best_d) {
					best_d = d;
					target_cell = &grid[r][c];
					target_r = r; target_c = c;
				}
			}
		}

		bool engaged = (target_cell != NULL);

		// 타깃이 있으면 이동 멈추고 공격 시도
		if (engaged) {
			if (enemies[i].atk_cooldown <= 0.0f) {
				// 타워에 피해
				target_cell->hp -= ENEMY_ATTACK_DAMAGE;
				enemies[i].atk_cooldown = ENEMY_ATTACK_COOLDOWN;

				// 타워 파괴 처리
				if (target_cell->hp <= 0) {
					target_cell->type = TOWER_EMPTY;
					target_cell->cooldown = 0.0f;
					target_cell->hp = 0;
					// 파괴되면 다음 프레임부터 다시 이동/다음 타워 탐색
				}
			}
			// engaged 상태에서는 이 프레임 이동하지 않음 (붙어서 때리는 느낌)
		}
		else {
			// 적을 왼쪽으로 이동
			enemies[i].x -= ENEMY_SPEED;

			// 적이 화면 왼쪽 끝에 도달하면
			if (enemies[i].x <= 0) {
				enemies[i].active = false; // 비활성화
				game_state.lives--;       // 생명력 감소
			}
		}
	}
}

void draw_game(ALLEGRO_BITMAP* background) {
	al_draw_bitmap(background, 0, 0, 0);

	// 타워 그리기
	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			float x1, y1, x2, y2;
			cell_rect(r, c, &x1, &y1, &x2, &y2);
			// 그리드 셀 그리기
			al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(80, 80, 80), 1.0f);

			if (grid[r][c].type != TOWER_EMPTY) {
				float cx = (x1 + x2) * 0.5f;
				float cy = (y1 + y2) * 0.5f;
				float radius = (CELL_W < CELL_H ? CELL_W : CELL_H) * 0.35f;

				if (grid[r][c].type == TOWER_ATTACK) {
					al_draw_filled_circle(cx, cy, radius, al_map_rgb(255, 100, 70)); // 빨간색 공격 타워
					al_draw_circle(cx, cy, radius, al_map_rgb(255, 250, 240), 2.0f);
				}
				else if (grid[r][c].type == TOWER_RESOURCE) {
					al_draw_filled_circle(cx, cy, radius, al_map_rgb(70, 255, 120)); // 초록색 자원 타워
					al_draw_circle(cx, cy, radius, al_map_rgb(240, 255, 250), 2.0f);
				}
				else if (grid[r][c].type == TOWER_TANK) {
					al_draw_filled_circle(cx, cy, radius, al_map_rgb(70, 120, 255)); // 파란색 탱커 타워
					al_draw_circle(cx, cy, radius, al_map_rgb(255, 250, 240), 2.0f);
				}
				int maxhp = tower_max_hp(grid[r][c].type);
				if (maxhp > 0) {
					float ratio = (float)grid[r][c].hp / (float)maxhp;
					float barw = CELL_W - 6.0f;
					float bx = x1 + 3.0f;
					float by = y1 + 2.0f;
					al_draw_filled_rectangle(bx, by, bx + barw, by + 2, al_map_rgb(40, 40, 40));
					al_draw_filled_rectangle(bx, by, bx + barw * ratio, by + 2, al_map_rgb(220, 120, 120));
				}
			}

		}
	}

	// 적 그리기
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		if (enemies[i].active) {
			// 본체
			al_draw_filled_rectangle(
				enemies[i].x - 5, enemies[i].y - 5,
				enemies[i].x + 5, enemies[i].y + 5,
				al_map_rgb(150, 50, 200)
			);
			// HP 바
			float eratio = (float)enemies[i].hp / (float)ENEMY_HP;
			if (eratio < 0.0f) eratio = 0.0f;
			if (eratio > 1.0f) eratio = 1.0f;
			float ex1 = enemies[i].x - 6, ey1 = enemies[i].y - 9;
			float ex2 = enemies[i].x + 6, ey2 = ey1 + 2;
			al_draw_filled_rectangle(ex1, ey1, ex2, ey2, al_map_rgb(40, 40, 40));
			al_draw_filled_rectangle(ex1, ey1, ex1 + (ex2 - ex1) * eratio, ey2, al_map_rgb(120, 220, 120));
		}
	}
	// ====== 공격 범위 시각화 ======
	// 1) show_all_ranges가 켜져 있으면 모든 공격 타워 범위 표시
	if (show_all_ranges) {
		for (int r = 0; r < ROWS; ++r) {
			for (int c = 0; c < COLS; ++c) {
				if (grid[r][c].type == TOWER_ATTACK) {
					float x1, y1, x2, y2; cell_rect(r, c, &x1, &y1, &x2, &y2);
					float cx = (x1 + x2) * 0.5f;
					float cy = (y1 + y2) * 0.5f;
					// 반투명 채움 + 외곽선
					al_draw_filled_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 100, 70, 40));
					al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 255, 120), 1.0f);
				}
			}
		}
	}
	// 2) 커서가 올려진 칸에 공격 타워가 있으면 해당 타워만 범위 표시
	else {
		if (grid[cursor_row][cursor_col].type == TOWER_ATTACK) {
			float x1, y1, x2, y2; cell_rect(cursor_row, cursor_col, &x1, &y1, &x2, &y2);
			float cx = (x1 + x2) * 0.5f;
			float cy = (y1 + y2) * 0.5f;
			al_draw_filled_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 100, 70, 40));
			al_draw_circle(cx, cy, ATTACK_TOWER_RANGE, al_map_rgba(255, 255, 255, 120), 1.0f);
		}
	}

	// 커서 표시
	float cx1, cy1, cx2, cy2;
	cell_rect(cursor_row, cursor_col, &cx1, &cy1, &cx2, &cy2);
	al_draw_rectangle(cx1 + 1, cy1 + 1, cx2 - 1, cy2 - 1, al_map_rgb(255, 255, 0), 2.0f);

	// UI 그리기
	al_draw_textf(game_font, al_map_rgb(0, 0, 0), 10, 10, 0, "Caffeine: %d", game_state.caffeine);
	al_draw_textf(game_font, al_map_rgb(0, 0, 0), 10, 20, 0, "LIVES: %d", game_state.lives);
	al_draw_textf(game_font, al_map_rgb(0, 0, 0), 10, 30, 0, "Stage %d/%d  Kills %d/%d", game_state.stage, MAX_STAGES, game_state.stage_kills, KILLS_TO_ADVANCE);
}

// 타워 설치
void place_tower(TowerType type) {
	if (grid[cursor_row][cursor_col].type == TOWER_EMPTY) {
		int cost = 0;
		if (type == TOWER_ATTACK) cost = ATTACK_TOWER_COST;
		else if (type == TOWER_RESOURCE) cost = RESOURCE_TOWER_COST;
		else if (type == TOWER_TANK) cost = TANK_TOWER_COST;

		if (game_state.caffeine >= cost) {
			game_state.caffeine -= cost;
			grid[cursor_row][cursor_col].type = type;
			grid[cursor_row][cursor_col].cooldown = 0; // 즉시 동작 가능하도록
			grid[cursor_row][cursor_col].hp = tower_max_hp(type);
		}
	}
}

// 타워 판매
void sell_tower() {
	Tower* cell = &grid[cursor_row][cursor_col];
	if (cell->type != TOWER_EMPTY) {
		int refund = 0;
		if (cell->type == TOWER_ATTACK) refund = ATTACK_TOWER_COST / 2;
		else if (cell->type == TOWER_RESOURCE) refund = RESOURCE_TOWER_COST / 2;
		else if (cell->type == TOWER_TANK) refund = TANK_TOWER_COST / 2;

		game_state.caffeine += refund;
		cell->type = TOWER_EMPTY;
		cell->hp = 0;
	}
}

// 디스플레이 초기화
void disp_init() {
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	disp = al_create_display(DISP_W, DISP_H);
	if (!disp) { fprintf(stderr, "Failed to create display.\n"); exit(1); }

	buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
	if (!buffer) { fprintf(stderr, "Failed to create buffer.\n"); exit(1); }
}

// 그리기 준비
void disp_pre_draw() { al_set_target_bitmap(buffer); }

// 화면에 최종 출력
void disp_post_draw() {
	al_set_target_backbuffer(disp);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	// 버퍼를 디스플레이 크기에 맞게 확대하여 그림
	al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0);
	al_flip_display();
}

int main(void) {
	if (!al_init()) { fprintf(stderr, "Failed to init Allegro.\n"); return 1; }
	al_install_keyboard();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	//al_init_ttf_addon();

	// 무작위 시드 초기화
	srand(time(NULL));

	disp_init();
	al_set_window_title(disp, "Tower Defense Extended");

	ALLEGRO_BITMAP* background = al_load_bitmap("bgr.png");
	if (!background) {
		// 배경 이미지가 없으면 단색으로 채움
		background = al_create_bitmap(BUFFER_W, BUFFER_H);
		al_set_target_bitmap(background);
		al_clear_to_color(al_map_rgb(50, 30, 0));
		fprintf(stderr, "Failed to load background image. Using a plain color.\n");
	}

	game_font = al_create_builtin_font();
	if (!game_font) { fprintf(stderr, "Failed to load font.\n"); return 1; }

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

	al_register_event_source(queue, al_get_display_event_source(disp));
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_keyboard_event_source());

	init_game_objects();

	bool running = true;
	bool redraw = true;
	al_start_timer(timer);

	while (running) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(queue, &ev);

		switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			running = false;
			break;
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			al_acknowledge_resize(disp);
			disp_post_draw(); // 리사이즈 시 화면 다시 그리기
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_ESCAPE: running = false; break;
			case ALLEGRO_KEY_UP:    if (cursor_row > 0) cursor_row--; break;
			case ALLEGRO_KEY_DOWN:  if (cursor_row < ROWS - 1) cursor_row++; break;
			case ALLEGRO_KEY_LEFT:  if (cursor_col > 0) cursor_col--; break;
			case ALLEGRO_KEY_RIGHT: if (cursor_col < COLS - 1) cursor_col++; break;
			case ALLEGRO_KEY_1: place_tower(TOWER_ATTACK); break;
			case ALLEGRO_KEY_2: place_tower(TOWER_RESOURCE); break;
			case ALLEGRO_KEY_3: place_tower(TOWER_TANK); break;
			case ALLEGRO_KEY_SPACE: sell_tower(); break;
			case ALLEGRO_KEY_R: show_all_ranges = !show_all_ranges; break;
			}
			break;

		case ALLEGRO_EVENT_TIMER:
			update_game(al_get_timer_speed(timer));

			// 적 생성 주기
			if (!game_state.cleared &&
				game_state.stage_kills < KILLS_TO_ADVANCE &&
				(al_get_time() - last_enemy_spawn_time > 1.0)) {
				spawn_enemy();
				last_enemy_spawn_time = al_get_time();
			}

			redraw = true;
			break;
		}

		if (redraw && al_is_event_queue_empty(queue)) {
			redraw = false;

			disp_pre_draw();
			al_clear_to_color(al_map_rgb(0, 0, 0));
			draw_game(background);
			disp_post_draw();
		}

		if (game_state.lives <= 0 || game_state.cleared) {
			running = false;
			if (game_state.cleared) {
				printf("You cleared all %d stages! Victory!\n", MAX_STAGES);
			}
			else {
				printf("Game Over!\n");
			}
		}
	}

	al_destroy_bitmap(background);
	al_destroy_font(game_font);
	al_destroy_event_queue(queue);
	al_destroy_timer(timer);
	al_destroy_bitmap(buffer);
	al_destroy_display(disp);

	return 0;
}
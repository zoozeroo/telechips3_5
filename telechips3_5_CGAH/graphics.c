// --- sprites for Sleep Defense ---

#define VIRUS_W  16
#define VIRUS_H  16

#define UNIT_W   24
#define UNIT_H   24

#define BEAN_W   12
#define BEAN_H   12

#define TEXT_W   16
#define TEXT_H   8

#define STUDENT_W 20
#define STUDENT_H 20

#define HPBAR_W   30
#define HPBAR_H   6
#define HPBAR_FRAMES 6

#define VIRUS_FRAMES 2
#define UNIT_FRAMES  2
#define TEXT_FRAMES  3

typedef struct SPRITES
{
    ALLEGRO_BITMAP* _sheet;

    // 적군
    ALLEGRO_BITMAP* virus_blue[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_yellow[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_red[VIRUS_FRAMES];
    ALLEGRO_BITMAP* virus_cyan[VIRUS_FRAMES];

    // 아군 유닛
    ALLEGRO_BITMAP* americano;            // 원거리 1프레임
    ALLEGRO_BITMAP* caffe_latte;          // 탱커 1프레임
    ALLEGRO_BITMAP* caramel_macchiato;    // 보조 1프레임

    // 자원
    ALLEGRO_BITMAP* coffee_bean;

    // 공격 텍스트
    ALLEGRO_BITMAP* answer_text[TEXT_FRAMES];

    // 학생
    ALLEGRO_BITMAP* student_sleeping;

    // 변신 유닛 애니메이션
    ALLEGRO_BITMAP* unit_ranged[UNIT_FRAMES];
    ALLEGRO_BITMAP* unit_tank[UNIT_FRAMES];
    ALLEGRO_BITMAP* unit_support[UNIT_FRAMES];

    // 체력바
    ALLEGRO_BITMAP* hpbar[HPBAR_FRAMES];

} SPRITES;

SPRITES sprites;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h)
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
    must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init()
{
    sprites._sheet = al_load_bitmap("spritesheet.png");
    must_init(sprites._sheet, "spritesheet");

    // --- 적군 (임의 좌표 예시) ---
    sprites.virus_blue[0] = sprite_grab(0, 0, VIRUS_W, VIRUS_H);
    sprites.virus_blue[1] = sprite_grab(16, 0, VIRUS_W, VIRUS_H);

    sprites.virus_yellow[0] = sprite_grab(32, 0, VIRUS_W, VIRUS_H);
    sprites.virus_yellow[1] = sprite_grab(48, 0, VIRUS_W, VIRUS_H);

    sprites.virus_red[0] = sprite_grab(0, 16, VIRUS_W, VIRUS_H);
    sprites.virus_red[1] = sprite_grab(16, 16, VIRUS_W, VIRUS_H);

    sprites.virus_cyan[0] = sprite_grab(32, 16, VIRUS_W, VIRUS_H);
    sprites.virus_cyan[1] = sprite_grab(48, 16, VIRUS_W, VIRUS_H);

    // --- 아군 유닛 기본 아이콘 ---
    sprites.americano = sprite_grab(0, 32, UNIT_W, UNIT_H);
    sprites.caffe_latte = sprite_grab(24, 32, UNIT_W, UNIT_H);
    sprites.caramel_macchiato = sprite_grab(48, 32, UNIT_W, UNIT_H);

    // --- 자원 ---
    sprites.coffee_bean = sprite_grab(0, 56, BEAN_W, BEAN_H);

    // --- 공격 텍스트 (랜덤 발사) ---
    sprites.answer_text[0] = sprite_grab(0, 72, TEXT_W, TEXT_H);
    sprites.answer_text[1] = sprite_grab(16, 72, TEXT_W, TEXT_H);
    sprites.answer_text[2] = sprite_grab(32, 72, TEXT_W, TEXT_H);

    // --- 학생 ---
    sprites.student_sleeping = sprite_grab(0, 88, STUDENT_W, STUDENT_H);

    // --- 변신 유닛 (애니메이션 2프레임씩) ---
    sprites.unit_ranged[0] = sprite_grab(24, 88, UNIT_W, UNIT_H);
    sprites.unit_ranged[1] = sprite_grab(48, 88, UNIT_W, UNIT_H);

    sprites.unit_tank[0] = sprite_grab(72, 88, UNIT_W, UNIT_H);
    sprites.unit_tank[1] = sprite_grab(96, 88, UNIT_W, UNIT_H);

    sprites.unit_support[0] = sprite_grab(120, 88, UNIT_W, UNIT_H);
    sprites.unit_support[1] = sprite_grab(144, 88, UNIT_W, UNIT_H);

    // --- 체력바 (6프레임) ---
    for (int i = 0; i < HPBAR_FRAMES; i++) {
        sprites.hpbar[i] = sprite_grab(0 + i * HPBAR_W, 120, HPBAR_W, HPBAR_H);
    }
}

void sprites_deinit()
{
    // 적군
    for (int i = 0; i < VIRUS_FRAMES; i++) {
        al_destroy_bitmap(sprites.virus_blue[i]);
        al_destroy_bitmap(sprites.virus_yellow[i]);
        al_destroy_bitmap(sprites.virus_red[i]);
        al_destroy_bitmap(sprites.virus_cyan[i]);
    }

    // 아군
    al_destroy_bitmap(sprites.americano);
    al_destroy_bitmap(sprites.caffe_latte);
    al_destroy_bitmap(sprites.caramel_macchiato);

    // 자원
    al_destroy_bitmap(sprites.coffee_bean);

    // 텍스트
    for (int i = 0; i < TEXT_FRAMES; i++) {
        al_destroy_bitmap(sprites.answer_text[i]);
    }

    // 학생
    al_destroy_bitmap(sprites.student_sleeping);

    // 변신 유닛
    for (int i = 0; i < UNIT_FRAMES; i++) {
        al_destroy_bitmap(sprites.unit_ranged[i]);
        al_destroy_bitmap(sprites.unit_tank[i]);
        al_destroy_bitmap(sprites.unit_support[i]);
    }

    // 체력바
    for (int i = 0; i < HPBAR_FRAMES; i++) {
        al_destroy_bitmap(sprites.hpbar[i]);
    }

    // 원본 시트
    al_destroy_bitmap(sprites._sheet);
}

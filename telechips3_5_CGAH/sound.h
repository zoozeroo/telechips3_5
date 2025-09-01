#pragma once
#include <stdbool.h>

typedef enum {
    SOUND_BUTTON_CLICK = 0,
    SOUND_PEOPLE2_COIN,
    SOUND_VIRUS3_TIMER,
    SOUND_VIRUS_EXPLODE,
    SOUND_LIFE,
    SOUND_BUY,
    SOUND_FAIL,
    SOUND_SUCCESS,
    SOUND_PEOPLE1_ATTACK,
    SOUND_VIRUS4_FREEZE,
    SOUND_COUNT  // 총 사운드 개수
} SoundType;

typedef enum {
    BGM_NONE = 0,
    BGM_OTHER,   // 메뉴, 랭킹, How to play 화면용
    BGM_GAME     // 게임 화면용
} BgmType;

bool sound_init(void);
void sound_cleanup(void);
void sound_play(SoundType type);
void sound_set_volume(float volume); // 0.0f ~ 1.0f

// 배경음악 관련
void bgm_play(BgmType bgm_type);
void bgm_stop(void);
void bgm_set_volume(float volume);
BgmType bgm_get_current(void);

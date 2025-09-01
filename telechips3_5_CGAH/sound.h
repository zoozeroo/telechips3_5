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
    SOUND_COUNT  // �� ���� ����
} SoundType;

typedef enum {
    BGM_NONE = 0,
    BGM_OTHER,   // �޴�, ��ŷ, How to play ȭ���
    BGM_GAME     // ���� ȭ���
} BgmType;

bool sound_init(void);
void sound_cleanup(void);
void sound_play(SoundType type);
void sound_set_volume(float volume); // 0.0f ~ 1.0f

// ������� ����
void bgm_play(BgmType bgm_type);
void bgm_stop(void);
void bgm_set_volume(float volume);
BgmType bgm_get_current(void);

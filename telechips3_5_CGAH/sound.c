#include "sound.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>

static ALLEGRO_SAMPLE* sounds[SOUND_COUNT] = { NULL };
static float master_volume = 1.0f;

// ������ǿ� ������
static ALLEGRO_SAMPLE* bgm_other = NULL;
static ALLEGRO_SAMPLE* bgm_game = NULL;
static ALLEGRO_SAMPLE_INSTANCE* current_bgm_instance = NULL;
static BgmType current_bgm_type = BGM_NONE;
static float bgm_volume = 0.3f;  // ��������� �� �� ����

static const char* sound_files[SOUND_COUNT] = {
    "button_click.mp3",
    "people2_coin.mp3",
    "virus3_timer.mp3",
    "virus3_explode.mp3",
    "life.mp3",
    "buy.mp3",
    "fail.mp3",
    "success.mp3",
    "people1_attack.mp3",
    "virus4_freeze.mp3"
};

bool sound_init(void) {
    if (!al_install_audio()) {
        fprintf(stderr, "Failed to install audio\n");
        return false;
    }

    if (!al_init_acodec_addon()) {
        fprintf(stderr, "Failed to init audio codec addon\n");
        return false;
    }

    if (!al_reserve_samples(16)) {
        fprintf(stderr, "Failed to reserve samples\n");
        return false;
    }

    // ���� ���� �ε�
    for (int i = 0; i < SOUND_COUNT; ++i) {
        sounds[i] = al_load_sample(sound_files[i]);
        if (!sounds[i]) {
            printf("Warning: Failed to load %s\n", sound_files[i]);
        }
    }

    // ������� ���� �ε�
    bgm_other = al_load_sample("bgm_other.mp3");
    if (!bgm_other) {
        printf("Warning: Failed to load bgm_other.mp3\n");
    }

    bgm_game = al_load_sample("bgm_game.mp3");
    if (!bgm_game) {
        printf("Warning: Failed to load bgm_game.mp3\n");
    }

    return true;
}

void sound_cleanup(void) {
    // ������� ����
    bgm_stop();

    if (bgm_other) {
        al_destroy_sample(bgm_other);
        bgm_other = NULL;
    }

    if (bgm_game) {
        al_destroy_sample(bgm_game);
        bgm_game = NULL;
    }

    // ȿ���� ����
    for (int i = 0; i < SOUND_COUNT; ++i) {
        if (sounds[i]) {
            al_destroy_sample(sounds[i]);
            sounds[i] = NULL;
        }
    }
}

void sound_play(SoundType type) {
    if (type < 0 || type >= SOUND_COUNT) return;
    if (!sounds[type]) return;

    al_play_sample(sounds[type], master_volume, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void sound_set_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    master_volume = volume;
}

void bgm_play(BgmType bgm_type) {
    // �̹� ���� BGM�� ��� ���̸� �ƹ��͵� ���� ����
    if (current_bgm_type == bgm_type && current_bgm_instance &&
        al_get_sample_instance_playing(current_bgm_instance)) {
        return;
    }

    // ���� BGM ����
    bgm_stop();

    // ���ο� BGM ����
    ALLEGRO_SAMPLE* target_sample = NULL;
    switch (bgm_type) {
    case BGM_OTHER:
        target_sample = bgm_other;
        break;
    case BGM_GAME:
        target_sample = bgm_game;
        break;
    default:
        return;
    }

    if (!target_sample) return;

    // ���� �ν��Ͻ� ���� �� ���
    current_bgm_instance = al_create_sample_instance(target_sample);
    if (current_bgm_instance) {
        al_set_sample_instance_playmode(current_bgm_instance, ALLEGRO_PLAYMODE_LOOP);
        al_set_sample_instance_gain(current_bgm_instance, bgm_volume);

        if (al_attach_sample_instance_to_mixer(current_bgm_instance, al_get_default_mixer())) {
            al_play_sample_instance(current_bgm_instance);
            current_bgm_type = bgm_type;
            printf("BGM started: %s\n", (bgm_type == BGM_OTHER) ? "bgm_other.mp3" : "bgm_game.mp3");
        }
        else {
            al_destroy_sample_instance(current_bgm_instance);
            current_bgm_instance = NULL;
            printf("Failed to attach BGM to mixer\n");
        }
    }
}

void bgm_stop(void) {
    if (current_bgm_instance) {
        al_stop_sample_instance(current_bgm_instance);
        al_destroy_sample_instance(current_bgm_instance);
        current_bgm_instance = NULL;
    }
    current_bgm_type = BGM_NONE;
}

void bgm_set_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    bgm_volume = volume;

    if (current_bgm_instance) {
        al_set_sample_instance_gain(current_bgm_instance, bgm_volume);
    }
}

BgmType bgm_get_current(void) {
    return current_bgm_type;
}
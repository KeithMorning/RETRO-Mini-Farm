#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <stdint.h>

#define GAME_W 320
#define GAME_H 240

typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t up;
    uint8_t down;
    uint8_t a;
    uint8_t b;
    uint8_t menu;
} Input;

extern int game_clock_mhz;

enum {
    GAME_SCENE_FARM = 0,
    GAME_SCENE_TOWN,
    GAME_SCENE_HOME,
    GAME_SCENE_SHOP,
    GAME_SCENE_MIRA,
    GAME_SCENE_TOM,
    GAME_SCENE_LILY,
    GAME_SCENE_BEACH,
    GAME_SCENE_MINE
};

void game_init(void);
void game_debug_warp(int scene, int tx, int ty);
void game_debug_time(int mins);
void game_debug_weather(int w);
void game_debug_season(int s);
void game_debug_give(int crop_id, int n);
void game_debug_select(int s);
void game_debug_tool(int kind);
void game_debug_unlock_mine(void);
void game_debug_fill_pets(void);
void game_debug_coins(int c);
void game_debug_points(int i, int v);
void game_debug_day(int d);
void game_debug_play(void);
void game_debug_fish_bite(void);
void game_debug_give_fish(int type, int n);
int game_debug_coins_now(void);
int game_debug_house_now(void);
void game_debug_set_heart(int i, int p);
void game_debug_give_crop(int id, int n);
int game_debug_points_get(int i);
int game_debug_story_get(int i);
int game_debug_target_tile(void);
int game_update(const Input *in);
void game_render(uint16_t *fb);

#endif

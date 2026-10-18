#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "game.h"

static uint16_t fb[GAME_W * GAME_H];

static void dump(const char *name)
{
    char path[256];
    snprintf(path, sizeof(path), "out/%s.bin", name);
    FILE *f = fopen(path, "wb");
    if (!f) {
        perror(path);
        exit(1);
    }
    fwrite(fb, sizeof(uint16_t), GAME_W * GAME_H, f);
    fclose(f);
}

static void step(Input *in, int frames)
{
    for (int i = 0; i < frames; i++) {
        game_update(in);
    }
}

static void tap(Input *in, uint8_t *field)
{
    *field = 1;
    game_update(in);
    *field = 0;
    game_update(in);
}

static void shot(Input *in, const char *name)
{
    game_render(fb);
    dump(name);
}

int main(void)
{
    mkdir("out", 0755);
    game_init();

    Input in;
    memset(&in, 0, sizeof(in));

    game_update(&in);
    shot(&in, "t0_title");

    tap(&in, &in.a);
    shot(&in, "t1_farm");
    in.down = 1; game_update(&in); in.down = 0;
    shot(&in, "t1b_player_front");

    /* Town, 10:00. */
    game_debug_time(600);
    game_debug_warp(GAME_SCENE_TOWN, 20, 13);
    game_update(&in);
    shot(&in, "t2_town");

    /* Shop interior + dialogue. */
    game_debug_time(720);
    game_debug_warp(GAME_SCENE_SHOP, 10, 5);
    game_update(&in);
    shot(&in, "t3_shop_inside");

    /* Talk to Mira in the plaza at 17:00. */
    game_debug_time(1020);
    game_debug_warp(GAME_SCENE_TOWN, 20, 15);
    step(&in, 1);
    game_debug_select(0);
    tap(&in, &in.a);
    shot(&in, "t4_dialogue");
    tap(&in, &in.b);

    game_debug_give(1, 1);
    game_debug_select(4);
    tap(&in, &in.a);
    shot(&in, "t5_gift");
    tap(&in, &in.b);

    /* Player home. */
    game_debug_warp(GAME_SCENE_HOME, 10, 8);
    game_update(&in);
    shot(&in, "t6_home");

    /* Farm book at the table. */
    game_debug_warp(GAME_SCENE_HOME, 8, 4);
    in.up = 1;
    step(&in, 1);
    in.up = 0;
    tap(&in, &in.a);
    shot(&in, "t12_status");
    tap(&in, &in.a);
    shot(&in, "t12b_crops");
    tap(&in, &in.b);

    /* Bed -> sleep. */
    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    in.up = 1;
    step(&in, 1);
    in.up = 0;
    tap(&in, &in.a);
    shot(&in, "t13_sleep");
    step(&in, 50);

    /* Home door -> farm. */
    game_debug_warp(GAME_SCENE_HOME, 10, 13);
    in.down = 1;
    step(&in, 1);
    in.down = 0;
    tap(&in, &in.a);
    step(&in, 30);
    shot(&in, "t14_exit");

    /* Mira home at night. */
    game_debug_time(1200);
    game_debug_warp(GAME_SCENE_MIRA, 10, 6);
    game_update(&in);
    shot(&in, "t7_mira_home");

    /* Winter / snow and spring / rain on the farm. */
    game_debug_time(600);
    game_debug_warp(GAME_SCENE_FARM, 9, 9);
    game_debug_season(3);
    game_debug_weather(2);
    game_update(&in);
    shot(&in, "t8_winter");

    game_debug_season(0);
    game_debug_weather(1);
    game_update(&in);
    shot(&in, "t9_rain");

    /* Door transition: farm house door -> home. */
    game_debug_weather(0);
    game_debug_warp(GAME_SCENE_FARM, 5, 8);
    in.up = 1;
    step(&in, 1);
    in.up = 0;
    tap(&in, &in.a);
    step(&in, 30);
    shot(&in, "t10_home_from_door");

    /* Spring ending. */
    game_debug_season(0);
    game_debug_day(28);
    game_debug_coins(600);
    game_debug_points(0, 60);
    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    in.up = 1; step(&in, 1); in.up = 0;
    tap(&in, &in.a);
    step(&in, 50);
    shot(&in, "t11_ending");
    game_debug_play();

    /* Town shop front at the entrance. */
    game_debug_season(0);
    game_debug_day(1);
    game_debug_coins(50);
    game_debug_time(600);
    game_debug_warp(GAME_SCENE_TOWN, 8, 13);
    step(&in, 1);
    shot(&in, "t15_town_shop");

    /* Season change overlay. */
    game_debug_play();
    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    game_debug_day(28);
    in.up = 1; step(&in, 1); in.up = 0;
    tap(&in, &in.a);
    step(&in, 50);
    shot(&in, "t16_summer");

    /* Year ending. */
    game_debug_play();
    for (int i = 0; i < 3; i++) game_debug_points(i, 60);
    game_debug_time(600);
    game_debug_day(28);
    game_debug_season(3);
    game_debug_coins(3000);
    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    in.up = 1; step(&in, 1); in.up = 0;
    tap(&in, &in.a);
    step(&in, 50);
    shot(&in, "t17_year_end");

    /* Shop counter. */
    game_debug_play();
    game_debug_season(0);
    game_debug_coins(50);
    game_debug_time(720);
    game_debug_warp(GAME_SCENE_SHOP, 10, 5);
    in.up = 1;
    step(&in, 1);
    in.up = 0;
    tap(&in, &in.a);
    shot(&in, "t18_shop_counter");
    tap(&in, &in.b);

    /* Tom and Lily homes at night. */
    game_debug_time(1200);
    game_debug_warp(GAME_SCENE_TOM, 10, 6);
    game_update(&in);
    shot(&in, "t19_tom_home");
    game_debug_warp(GAME_SCENE_LILY, 10, 6);
    game_update(&in);
    shot(&in, "t20_lily_home");

    /* Summer and fall palettes. */
    game_debug_warp(GAME_SCENE_FARM, 9, 9);
    game_debug_weather(0);
    game_debug_season(1);
    game_update(&in);
    shot(&in, "t21_summer");
    game_debug_weather(0);
    game_debug_season(2);
    game_update(&in);
    shot(&in, "t22_fall");

    /* Town shop door transition. */
    game_debug_play();
    game_debug_time(600);
    game_debug_season(0);
    game_debug_warp(GAME_SCENE_TOWN, 7, 10);
    in.up = 1;
    step(&in, 1);
    in.up = 0;
    tap(&in, &in.a);
    step(&in, 30);
    shot(&in, "t23_shop_door");

    /* Forgiving interactions: talk prompt, A alone, door prompt. */
    game_debug_play();
    game_debug_season(0);
    game_debug_weather(0);
    game_debug_time(600);
    game_debug_warp(GAME_SCENE_SHOP, 16, 5);
    step(&in, 1);
    shot(&in, "t24_talk_prompt");
    tap(&in, &in.a);
    shot(&in, "t25_talk_no_facing");
    tap(&in, &in.b);

    game_debug_play();
    game_debug_warp(GAME_SCENE_FARM, 5, 8);
    step(&in, 1);
    shot(&in, "t26_door_prompt");
    tap(&in, &in.a);
    step(&in, 30);
    shot(&in, "t27_door_no_facing");

    /* Front-facing character. */
    game_debug_play();
    game_debug_warp(GAME_SCENE_FARM, 9, 9);
    in.down = 1;
    step(&in, 1);
    in.down = 0;
    shot(&in, "t28_front");

    /* Shop quantity selector. */
    game_debug_play();
    game_debug_time(600);
    game_debug_season(0);
    game_debug_warp(GAME_SCENE_FARM, 9, 9);
    in.up = 1; step(&in, 1); in.up = 0; /* face the stand */
    tap(&in, &in.a); /* open stand */
    tap(&in, &in.right);
    shot(&in, "t30_shop_qty");
    tap(&in, &in.a);
    shot(&in, "t31_shop_buy5");
    tap(&in, &in.b);

    /* Season countdown. */
    game_debug_play();
    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    game_debug_day(25);
    in.up = 1; step(&in, 1); in.up = 0;
    tap(&in, &in.a);
    step(&in, 50);
    shot(&in, "t29_countdown");

    /* Auto-enter a door by walking into it (no A). */
    game_debug_play();
    game_debug_season(0);
    game_debug_time(600);
    game_debug_warp(GAME_SCENE_FARM, 5, 8);
    in.up = 1;
    step(&in, 1);
    in.up = 0;
    step(&in, 30);
    shot(&in, "t32_auto_door");

    /* Gift while holding a seed: any produce should still work. */
    game_debug_play();
    game_debug_time(600);
    game_debug_season(0);
    game_debug_warp(GAME_SCENE_TOWN, 19, 15);
    game_debug_give(3, 1);
    game_debug_select(6);
    step(&in, 1);
    printf("points %d %d %d story %d %d %d\n", game_debug_points_get(0),game_debug_points_get(1),game_debug_points_get(2),game_debug_story_get(0),game_debug_story_get(1),game_debug_story_get(2));
    shot(&in, "t33_gift_prompt");
    tap(&in, &in.b);
    shot(&in, "t34_gift_seed");

    game_debug_play();
    game_debug_time(600);
    game_debug_season(0);
    game_debug_warp(GAME_SCENE_BEACH, 20, 3);
    step(&in, 1);
    shot(&in, "t35_beach");

    game_debug_play();
    game_debug_time(1200);
    game_debug_season(0);
    game_debug_warp(GAME_SCENE_FARM, 9, 9);
    step(&in, 1);
    shot(&in, "t36_dusk");

    printf("wrote frames to sim/out\n");
    return 0;
}

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
    fwrite(fb, 2, GAME_W * GAME_H, f);
    fclose(f);
}

static void step(Input *in, int n)
{
    for (int i = 0; i < n; i++) game_update(in);
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
    (void)in;
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
    tap(&in, &in.a);          /* title -> play */
    tap(&in, &in.a);          /* stand -> shop */
    tap(&in, &in.a);          /* buy carrot */
    tap(&in, &in.b);          /* exit shop */

    game_debug_warp(GAME_SCENE_FARM, 13, 9);
    game_debug_select(0);
    tap(&in, &in.a);          /* plant */
    shot(&in, "g1_plant");

    game_debug_tool(2);       /* can */
    tap(&in, &in.a);          /* water */
    shot(&in, "g2_water");

    tap(&in, &in.menu);       /* sleep */
    step(&in, 50);
    shot(&in, "g3_day2");

    game_debug_tool(2);
    tap(&in, &in.a);          /* water */
    tap(&in, &in.menu);
    for (int i = 0; i < 6; i++) tap(&in, &in.down);
    tap(&in, &in.a);          /* sleep */
    step(&in, 50);
    shot(&in, "g4_day3");

    tap(&in, &in.a);          /* harvest */
    shot(&in, "g5_harvest");

    game_debug_select(0);     /* select carrot seed */
    tap(&in, &in.a);          /* replant */
    shot(&in, "g5b_replant");

    game_debug_warp(GAME_SCENE_FARM, 8, 9);
    in.down = 1; step(&in, 1); in.down = 0;
    tap(&in, &in.a);          /* shipping bin panel */
    shot(&in, "g6_bin");
    tap(&in, &in.a);          /* ship all */
    shot(&in, "g6b_shipped");

    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    in.up = 1; step(&in, 1); in.up = 0;
    tap(&in, &in.a);          /* bed */
    step(&in, 50);
    shot(&in, "g7_summary");
    tap(&in, &in.a);
    shot(&in, "g8_coins");

    printf("gameplay test done\n");
    return 0;
}

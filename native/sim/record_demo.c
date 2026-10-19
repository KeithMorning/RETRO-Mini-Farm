/* Capture a short gameplay reel from the host build.
   Writes 320x240 RGB24 frames to sim/out/demo.rgb. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "game.h"

static uint16_t fb[GAME_W * GAME_H];
static FILE *raw;

static void emit(void)
{
    game_render(fb);
    for (int y = 0; y < GAME_H; y++) {
        for (int x = 0; x < GAME_W; x++) {
            uint16_t v = fb[x * GAME_H + y];
            int r = ((v >> 11) & 31) * 255 / 31;
            int g = ((v >> 5) & 63) * 255 / 63;
            int b = (v & 31) * 255 / 31;
            fputc(r, raw); fputc(g, raw); fputc(b, raw);
        }
    }
}

static void cap(Input *in, int n)
{
    for (int i = 0; i < n; i++) {
        game_update(in);
        emit();
    }
}

static void tap(Input *in, uint8_t *field)
{
    *field = 1;
    game_update(in);
    emit();
    *field = 0;
    game_update(in);
    emit();
}

static void press(Input *in, uint8_t *field)
{
    *field = 1;
    game_update(in);
    emit();
    *field = 0;
    game_update(in);
}

int main(void)
{
    mkdir("out", 0755);
    raw = fopen("out/demo.rgb", "wb");
    if (!raw) return 1;

    Input in;
    memset(&in, 0, sizeof(in));
    game_init();

    /* Full intro animation at capture speed, then hold the title card. */
    for (int i = 0; i < 210; i++) {
        game_update(&in);
        emit();
    }

    /* Start. */
    tap(&in, &in.a);

    /* Farm: till, plant, water. */
    game_debug_warp(GAME_SCENE_FARM, 13, 9);
    game_debug_tool(1);
    cap(&in, 8);
    tap(&in, &in.a);
    cap(&in, 10);
    game_debug_select(0);
    tap(&in, &in.a);
    cap(&in, 10);
    game_debug_tool(2);
    tap(&in, &in.a);
    cap(&in, 16);

    /* Town and dialogue. */
    game_debug_time(10 * 60);
    game_debug_warp(GAME_SCENE_TOWN, 19, 16);
    in.up = 1; game_update(&in); emit(); in.up = 0;
    cap(&in, 8);
    tap(&in, &in.a);
    cap(&in, 48);
    tap(&in, &in.b);
    cap(&in, 6);

    /* Bag. */
    game_debug_warp(GAME_SCENE_FARM, 13, 9);
    in.menu = 1; game_update(&in); emit(); in.menu = 0;
    in.down = 1; game_update(&in); emit(); in.down = 0;
    tap(&in, &in.a);
    cap(&in, 45);
    tap(&in, &in.b);
    cap(&in, 6);

    /* Beach fishing. */
    game_debug_rod();
    game_debug_tool(3);
    game_debug_warp(GAME_SCENE_BEACH, 20, 15);
    in.up = 1; game_update(&in); emit(); in.up = 0;
    tap(&in, &in.a);
    cap(&in, 18);
    game_debug_fish_bite();
    cap(&in, 8);
    tap(&in, &in.a);
    cap(&in, 55);
    tap(&in, &in.b);
    cap(&in, 6);

    /* Mine floor 2. */
    game_debug_unlock_mine();
    game_debug_tool(4);
    game_debug_warp(GAME_SCENE_MINE, 10, 3);
    in.up = 1; game_update(&in); emit(); in.up = 0;
    tap(&in, &in.a);
    cap(&in, 55);

    /* Pets and dino. */
    game_debug_fill_pets();
    game_debug_warp(GAME_SCENE_HOME, 10, 10);
    cap(&in, 50);

    /* Year-end goal. */
    game_init();
    memset(&in, 0, sizeof(in));
    game_update(&in);
    tap(&in, &in.a);
    game_debug_unlock_mine();
    for (int i = 0; i < 3; i++) game_debug_points(i, 60);
    game_debug_season(3);
    game_debug_day(28);
    game_debug_warp(GAME_SCENE_HOME, 3, 5);
    in.up = 1; game_update(&in); emit(); in.up = 0;
    tap(&in, &in.a);
    cap(&in, 75);

    fclose(raw);
    printf("wrote out/demo.rgb\n");
    return 0;
}

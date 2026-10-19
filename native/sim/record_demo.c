/* Capture a focused gameplay reel for the Douyin trailer.
   The first 210 frames are the full startup animation.
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

static void face(Input *in, uint8_t *dir)
{
    *dir = 1;
    game_update(in);
    emit();
    *dir = 0;
}

int main(void)
{
    mkdir("out", 0755);
    raw = fopen("out/demo.rgb", "wb");
    if (!raw) return 1;

    Input in;
    memset(&in, 0, sizeof(in));
    game_init();

    /* Full startup animation at normal speed. */
    for (int i = 0; i < 210; i++) {
        game_update(&in);
        emit();
    }
    tap(&in, &in.a);

    /* Farm: face the camera, till, plant, water. */
    game_debug_warp(GAME_SCENE_FARM, 13, 7);
    game_debug_tool(1);
    face(&in, &in.down);
    cap(&in, 10);
    tap(&in, &in.a);
    cap(&in, 15);
    game_debug_select(0);
    tap(&in, &in.a);
    cap(&in, 15);
    game_debug_tool(2);
    tap(&in, &in.a);
    cap(&in, 23);

    /* Fishing minigame. */
    game_debug_rod();
    game_debug_tool(3);
    game_debug_warp(GAME_SCENE_BEACH, 20, 15);
    in.up = 1; game_update(&in); emit(); in.up = 0;
    tap(&in, &in.a);
    cap(&in, 20);
    game_debug_fish_bite();
    cap(&in, 10);
    tap(&in, &in.a);
    cap(&in, 60);
    tap(&in, &in.b);
    cap(&in, 5);

    /* Town: front-facing player, Mira portrait, story. */
    game_debug_time(10 * 60);
    game_debug_warp(GAME_SCENE_TOWN, 19, 16);
    face(&in, &in.down);
    cap(&in, 10);
    tap(&in, &in.a);
    cap(&in, 80);
    tap(&in, &in.b);
    cap(&in, 5);

    /* Pets and the little dinosaur. */
    game_debug_fill_pets();
    game_debug_warp(GAME_SCENE_HOME, 10, 10);
    face(&in, &in.down);
    cap(&in, 99);

    /* Mine floor 2 and a sword swing. */
    game_debug_unlock_mine();
    game_debug_tool(4);
    game_debug_warp(GAME_SCENE_MINE, 10, 3);
    in.up = 1; game_update(&in); emit(); in.up = 0;
    tap(&in, &in.a);
    cap(&in, 50);
    game_debug_warp(GAME_SCENE_MINE, 5, 5);
    face(&in, &in.down);
    tap(&in, &in.a);
    cap(&in, 63);

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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "game.h"

static uint16_t fb[GAME_W * GAME_H];

static void dump(const char *name)
{
    char path[128];
    snprintf(path, sizeof(path), "out/%s.bin", name);
    FILE *f = fopen(path, "wb");
    fwrite(fb, 2, GAME_W * GAME_H, f);
    fclose(f);
}

int main(void)
{
    mkdir("out", 0755);
    remove("save.dat");

    Input in;
    memset(&in, 0, sizeof(in));

    game_init();
    in.a = 1; game_update(&in); in.a = 0;
    game_debug_coins(777);
    game_debug_day(5);
    game_debug_season(2);
    game_debug_warp(GAME_SCENE_HOME, 10, 8);
    in.menu = 1; game_update(&in); in.menu = 0;
    game_render(fb);
    dump("save_before");

    game_init();
    game_debug_play();
    game_render(fb);
    dump("save_after");
    printf("save test done\n");
    return 0;
}

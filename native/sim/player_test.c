#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "game.h"
static uint16_t fb[GAME_W*GAME_H];
static void dump(const char*n){char p[128];snprintf(p,sizeof p,"out/%s.bin",n);FILE*f=fopen(p,"wb");fwrite(fb,2,GAME_W*GAME_H,f);fclose(f);}
static void shot(const char*n){game_render(fb);dump(n);}
int main(void){
 mkdir("out",0755); game_init(); Input in; memset(&in,0,sizeof in); game_update(&in);
 in.a=1; game_update(&in); in.a=0; game_update(&in);
 game_debug_warp(GAME_SCENE_FARM,14,14);
 in.down=1; game_update(&in); in.down=0;
 shot("pl_front");
 in.up=1; game_update(&in); in.up=0;
 shot("pl_back");
 printf("player test done\n"); return 0;
}

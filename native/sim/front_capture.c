#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "game.h"
static uint16_t fb[GAME_W*GAME_H];
int main(void){
 mkdir("out",0755); game_init(); Input in; memset(&in,0,sizeof in);
 game_update(&in); in.a=1; game_update(&in); in.a=0;
 game_debug_warp(GAME_SCENE_FARM,13,8);
 in.down=1; game_update(&in); in.down=0;
 game_render(fb);
 FILE*f=fopen("out/front.bin","wb"); fwrite(fb,2,GAME_W*GAME_H,f); fclose(f);
 printf("front frame\n"); return 0;
}

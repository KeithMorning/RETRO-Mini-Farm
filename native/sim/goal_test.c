#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "game.h"
static uint16_t fb[GAME_W*GAME_H];
static void dump(const char*n){char p[128];snprintf(p,sizeof p,"out/%s.bin",n);FILE*f=fopen(p,"wb");fwrite(fb,2,GAME_W*GAME_H,f);fclose(f);}
static void step(Input*in,int n){for(int i=0;i<n;i++)game_update(in);}
static void tap(Input*in,uint8_t*f){*f=1;game_update(in);*f=0;game_update(in);}
int main(void){
 mkdir("out",0755); game_init(); Input in; memset(&in,0,sizeof in); game_update(&in);
 tap(&in,&in.a);
 game_debug_unlock_mine();
 for(int i=0;i<3;i++) game_debug_points(i,60);
 game_debug_coins(0);
 game_debug_season(3); game_debug_day(28); game_debug_warp(GAME_SCENE_HOME,3,5);
 in.up=1; step(&in,1); in.up=0; tap(&in,&in.a); step(&in,50);
 game_render(fb); dump("g_year_end");
 printf("goal test done\n"); return 0;
}

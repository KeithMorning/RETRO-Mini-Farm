#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "game.h"
static uint16_t fb[GAME_W*GAME_H];
static void dump(const char*n){char p[128];snprintf(p,sizeof p,"out/%s.bin",n);FILE*f=fopen(p,"wb");fwrite(fb,2,GAME_W*GAME_H,f);fclose(f);}
static void step(Input*in,int n){for(int i=0;i<n;i++)game_update(in);}
static void tap(Input*in,uint8_t*f){*f=1;game_update(in);*f=0;game_update(in);}
static void shot(const char*n){game_render(fb);dump(n);}
int main(void){
 mkdir("out",0755); game_init(); Input in; memset(&in,0,sizeof in); game_update(&in);
 tap(&in,&in.a); game_debug_unlock_mine();
 game_debug_time(18*60+30); game_debug_warp(GAME_SCENE_TOWN,20,16);
 in.up=1; step(&in,1); in.up=0;
 shot("e1_fountain");
 tap(&in,&in.a); shot("e2_campfire");
 tap(&in,&in.b);
 game_debug_time(20*60); tap(&in,&in.a); shot("e3_wish");
 game_debug_warp(GAME_SCENE_FARM,7,9);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); shot("e4_letter");
 tap(&in,&in.b);
 game_debug_time(18*60+45); game_debug_warp(GAME_SCENE_BEACH,30,15);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); shot("e5_dolphin");
 game_debug_fill_pets(); game_debug_time(21*60); game_debug_warp(GAME_SCENE_HOME,3,5);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); shot("e6_cat_bed");
 printf("easter test done\n"); return 0;
}

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
 tap(&in,&in.a); game_debug_coins(500);
 game_debug_set_heart(0,40); game_debug_give_crop(1,3);
 game_debug_warp(GAME_SCENE_MIRA,10,7);
 in.up=1; step(&in,1); in.up=0;
 shot("st1_marker");
 tap(&in,&in.a); shot("st2_start");
 for(int i=0;i<4;i++) tap(&in,&in.a);
 tap(&in,&in.a); shot("st3_done");
 for(int i=0;i<3;i++) tap(&in,&in.a);
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); shot("st4_journal");
 printf("story test done\n"); return 0;
}

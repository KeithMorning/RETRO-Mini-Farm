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
 mkdir("out",0755); remove("save.dat"); game_init(); Input in; memset(&in,0,sizeof in); game_update(&in);
 tap(&in,&in.a);
 tap(&in,&in.menu); shot("m1_menu");
 tap(&in,&in.down); tap(&in,&in.a); shot("m2_bag");
 tap(&in,&in.right); shot("m2b_bag_right");
 tap(&in,&in.down); shot("m2c_bag_down");
 tap(&in,&in.b);
 tap(&in,&in.menu); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.a); shot("m9_hearts");
 tap(&in,&in.a); tap(&in,&in.a); shot("m10_stories");
 tap(&in,&in.b);
 tap(&in,&in.menu); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.a); shot("m3_saved");
 game_debug_warp(GAME_SCENE_HOME,10,8);
 tap(&in,&in.menu); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.a); shot("m8_house_menu");
 game_debug_warp(GAME_SCENE_HOME,3,5);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.menu);
 for(int i=0;i<6;i++) tap(&in,&in.down);
 tap(&in,&in.a); step(&in,50); shot("m4_sleep");
 game_debug_warp(GAME_SCENE_HOME,10,8);
 tap(&in,&in.menu);
 for(int i=0;i<6;i++) tap(&in,&in.down);
 tap(&in,&in.a); shot("m7_go_bed");
 tap(&in,&in.menu);
 for(int i=0;i<5;i++) tap(&in,&in.down);
 tap(&in,&in.a); shot("m5_load_confirm");
 tap(&in,&in.a); shot("m6_loaded");
 printf("menu test done\n"); return 0;
}

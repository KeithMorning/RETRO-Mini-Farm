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
 tap(&in,&in.a);
 game_debug_coins(99999);
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a);
 shot("u1_upgrade"); printf("after open: coins=%d house=%d\n",game_debug_coins_now(),game_debug_house_now());
 tap(&in,&in.a); shot("u2_house1"); printf("after h1: coins=%d house=%d\n",game_debug_coins_now(),game_debug_house_now());
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a); /* reopen table -> page2 */
 tap(&in,&in.down); tap(&in,&in.a); shot("u3_can1"); printf("after can1: coins=%d house=%d\n",game_debug_coins_now(),game_debug_house_now());
 game_debug_warp(GAME_SCENE_FARM,9,9); shot("u4_house_exterior");
 game_debug_warp(GAME_SCENE_HOME,10,8); shot("u5_house_inside");

 /* second house level */
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a);
 tap(&in,&in.a);
 game_debug_warp(GAME_SCENE_FARM,9,9); shot("u6_house2_exterior"); printf("after h2: coins=%d house=%d\n",game_debug_coins_now(),game_debug_house_now());
 game_debug_warp(GAME_SCENE_HOME,10,8); shot("u7_house2_inside");

 /* level 3 */
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a);
 game_debug_warp(GAME_SCENE_FARM,9,9); shot("u8_house3_exterior"); printf("after h3: coins=%d house=%d\n",game_debug_coins_now(),game_debug_house_now());
 game_debug_warp(GAME_SCENE_HOME,10,8); shot("u9_house3_inside");

 /* level 4 */
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a);
 game_debug_warp(GAME_SCENE_FARM,9,9); shot("u10_house4_exterior"); printf("after h4: coins=%d house=%d\n",game_debug_coins_now(),game_debug_house_now());
 game_debug_warp(GAME_SCENE_HOME,10,8); shot("u11_house4_inside");
 game_debug_coins(99999);
 game_debug_unlock_mine();
 game_debug_rod();
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a); /* open upgrade */
 shot("u12_tools");
 tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.a); /* rod */
 shot("u13_rod");
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a); tap(&in,&in.a);
 tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.down); tap(&in,&in.a); /* sword */
 shot("u14_sword");
 printf("upgrade test done\n"); return 0;
}

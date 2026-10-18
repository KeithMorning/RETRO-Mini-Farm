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
 tap(&in,&in.a); game_debug_coins(1000);
 game_debug_warp(GAME_SCENE_SHOP,18,6);
 in.up=1; step(&in,1); in.up=0;
 shot("p1_pet_sign");
 tap(&in,&in.a); shot("p2_pet_shop");
 tap(&in,&in.a); shot("p3_cat_bought");
 game_debug_warp(GAME_SCENE_SHOP,18,6);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); tap(&in,&in.down); tap(&in,&in.a); shot("p3b_chicken_bought");
 game_debug_warp(GAME_SCENE_HOME,12,11); shot("p4_cat_home");
 game_debug_warp(GAME_SCENE_HOME,15,7);
 in.up=1; step(&in,1); in.up=0;
 shot("p5_bowl");
 tap(&in,&in.a); shot("p6_fed");
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); shot("p7_pet_book");
 game_debug_fill_pets();
 game_debug_warp(GAME_SCENE_HOME,12,12); shot("p8_dino");
 game_debug_warp(GAME_SCENE_HOME,8,4);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); shot("p9_pets_book");
 printf("pet test done\n"); return 0;
}

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
 game_debug_warp(GAME_SCENE_BEACH,6,8);
 in.up=1; step(&in,1); in.up=0;
 shot("b1_rod_prompt");
 tap(&in,&in.a); shot("b2_rod_bought");
 game_debug_tool(3); /* rod */
 game_debug_warp(GAME_SCENE_BEACH,24,15);
 in.down=1; step(&in,1); in.down=0;
 shot("b3_pier");
 printf("target tile=%d\n", game_debug_target_tile());
 tap(&in,&in.a); shot("b4_wait");
 game_debug_fish_bite(); shot("b5_bite");
 tap(&in,&in.a); shot("b6_game");
 in.a=1; step(&in,300); in.a=0; shot("b7_catch");
 game_debug_give_fish(1,2);
 tap(&in,&in.menu); tap(&in,&in.down); tap(&in,&in.a); shot("b12_fish_bag");
 tap(&in,&in.b);
 game_debug_warp(GAME_SCENE_FARM,8,9);
 in.down=1; step(&in,1); in.down=0;
 tap(&in,&in.a); shot("b13_bin_fish");
 tap(&in,&in.a);
 game_debug_warp(GAME_SCENE_HOME,3,5);
 in.up=1; step(&in,1); in.up=0;
 tap(&in,&in.a); step(&in,50); shot("b14_fish_summary");
 game_debug_warp(GAME_SCENE_BEACH,14,21);
 in.down=1; step(&in,1); in.down=0;
 shot("b8_shell");
 tap(&in,&in.a); shot("b9_shell_picked");
 game_debug_warp(GAME_SCENE_TOWN,20,27);
 in.down=1; step(&in,1); in.down=0;
 step(&in,30); shot("b10_beach_from_town");
 in.up=1; step(&in,1); in.up=0;
 step(&in,30); shot("b11_town_from_beach");
 printf("beach test done\n"); return 0;
}

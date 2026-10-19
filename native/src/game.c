#include "game.h"
#include "sound.h"
#include "save.h"
#include <string.h>

#define TILE 16
#define FARM_W 40
#define FARM_H 30
#define INT_W 20
#define INT_H 15
#define PLOTS 24

#define RGB(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))

/* Outdoor palette, switched by season. */
static uint16_t C_GRASS, C_GRASS_D, C_GRASS_L;
static uint16_t C_SOIL, C_SOIL_D;
static uint16_t C_WATER, C_WATER_D, C_WATER_L;
static uint16_t C_PATH, C_PATH_D;
static uint16_t C_LEAF, C_LEAF_L, C_LEAF_D;

/* Fixed palette. */
static const uint16_t C_WOOD = RGB(112, 78, 44);
static const uint16_t C_WOOD_L = RGB(144, 106, 58);
static const uint16_t C_ROOF = RGB(146, 62, 44);
static const uint16_t C_ROOF_D = RGB(104, 42, 32);
static const uint16_t C_FLOOR = RGB(150, 112, 72);
static const uint16_t C_FLOOR_D = RGB(118, 84, 52);
static const uint16_t C_WALL_IN = RGB(196, 176, 146);
static const uint16_t C_WALL_IN_D = RGB(150, 128, 104);
static const uint16_t C_WHITE = RGB(255, 246, 226);
static const uint16_t C_BLACK = RGB(0, 0, 0);
static const uint16_t C_HUD = RGB(54, 38, 28);
static const uint16_t C_HUD_L = RGB(122, 86, 54);
static const uint16_t C_GOLD = RGB(246, 198, 92);
static const uint16_t C_ENERGY = RGB(112, 196, 96);
static const uint16_t C_ENERGY_D = RGB(70, 140, 66);
static const uint16_t C_SAND = RGB(224, 200, 150);
static const uint16_t C_SAND_D = RGB(184, 158, 112);
static const uint16_t C_FENCE = RGB(148, 104, 58);
static const uint16_t C_FENCE_L = RGB(176, 130, 76);
static const uint16_t C_FENCE_D = RGB(98, 64, 36);
static const uint16_t C_BRICK = RGB(158, 86, 68);
static const uint16_t C_BRICK_D = RGB(112, 56, 46);
static const uint16_t C_ROSE = RGB(240, 136, 164);

enum {
    T_GRASS, T_GRASS2, T_SOIL, T_SOIL_W, T_WATER, T_PATH, T_TREE,
    T_WALL, T_WINDOW, T_ROOF, T_DOOR, T_FLOWER, T_FENCE, T_GATE,
    T_SAND, T_CHIMNEY, T_STAND, T_BIN, T_LAMP, T_FOUNTAIN,
    T_FLOOR, T_WALL_IN, T_BED, T_TABLE, T_COUNTER, T_SHELF,
    T_RUG, T_PLANT, T_STOVE, T_DOOR_IN, T_WINDOW_IN,
    T_CHAIR, T_BOOKSHELF, T_CRATE, T_AWNING, T_SIGN,
    T_SCARECROW, T_BARREL, T_MAILBOX, T_BENCH,
    T_PIER, T_ROCK, T_PAINTING, T_CLOCK,
    T_ROOF2, T_WALL2, T_FLOOR2, T_WALL_IN2, T_BOWL, T_ROOF3,
    T_MINE_FLOOR, T_MINE_WALL, T_ORE, T_LADDER, T_BOWL2, T_BED2,
    T_COUNT
};

enum { SC_FARM, SC_TOWN, SC_HOME, SC_SHOP, SC_MIRA, SC_TOM, SC_LILY, SC_BEACH, SC_MINE, SC_COUNT };
enum { ST_TITLE, ST_PLAY, ST_WIN };

typedef struct {
    const char *name;
    uint8_t days;
    uint8_t regrow;
    uint8_t price;
    uint16_t sell;
    uint16_t fruit;
} CropDef;

static const CropDef crops[16] = {
    {"carrot",  2, 0, 10, 25, RGB(240, 140, 60)},
    {"potato",  3, 0, 15, 40, RGB(180, 132, 82)},
    {"cabbage", 4, 0, 20, 70, RGB(150, 200, 110)},
    {"berry",   5, 2, 30, 60, RGB(238, 70, 90)},
    {"corn",    5, 2, 25, 45, RGB(250, 220, 90)},
    {"tomato",  4, 2, 20, 50, RGB(230, 80, 60)},
    {"bean",    6, 2, 30, 40, RGB(120, 190, 90)},
    {"melon",   6, 0, 35, 90, RGB(120, 210, 140)},
    {"pumpkin", 5, 0, 25, 80, RGB(240, 140, 44)},
    {"beet",     4, 0, 20, 55, RGB(170, 60, 90)},
    {"grape",    6, 3, 30, 70, RGB(140, 80, 180)},
    {"yam",      5, 3, 30, 45, RGB(190, 150, 90)},
    {"pea",      4, 2, 20, 50, RGB(140, 210, 100)},
    {"root",     3, 0, 15, 35, RGB(210, 170, 120)},
    {"fossil",   0, 0, 120, 300, RGB(210, 196, 164)},
    {"egg",      0, 0, 50, 50, RGB(246, 240, 218)},
};
static const uint8_t season_crop_count[4] = {4, 4, 4, 2};

static const char *season_name[4] = {"spring", "summer", "fall", "winter"};

typedef struct {
    uint16_t start;
    uint8_t scene;
    uint8_t x, y;
} Sched;

typedef struct {
    const char *name;
    uint8_t home;
    uint8_t favorite;
    const char *lines[3][2];
    const char *season[4];
    const char *first[2];
    const char *rain[2];
    const char *snow[2];
} NpcDef;

typedef struct {
    int16_t scene;
    int16_t x, y;
    uint8_t dir;
    uint8_t frame;
    uint8_t moving;
    int16_t wx, wy;
    uint16_t wait;
    uint8_t wander;
    uint8_t step;
} Actor;

typedef struct {
    uint8_t scene, x, y, target, tx, ty, dir;
} Door;

static const Door doors[] = {
    {SC_FARM, 5, 7, SC_HOME, 10, 13, 1},
    {SC_FARM, 6, 7, SC_HOME, 10, 13, 1},
    {SC_FARM, 38, 14, SC_TOWN, 2, 14, 3},
    {SC_HOME, 10, 14, SC_FARM, 5, 8, 0},
    {SC_TOWN, 1, 14, SC_FARM, 37, 14, 2},
    {SC_TOWN, 7, 9, SC_SHOP, 10, 13, 1},
    {SC_TOWN, 15, 9, SC_MIRA, 10, 13, 1},
    {SC_TOWN, 25, 9, SC_TOM, 10, 13, 1},
    {SC_TOWN, 8, 22, SC_LILY, 10, 13, 1},
    {SC_TOWN, 20, 28, SC_BEACH, 20, 3, 0},
    {SC_BEACH, 20, 2, SC_TOWN, 20, 27, 1},
    {SC_TOWN, 20, 1, SC_MINE, 10, 12, 0},
    {SC_MINE, 10, 13, SC_TOWN, 20, 2, 1},
    {SC_SHOP, 10, 14, SC_TOWN, 7, 10, 0},
    {SC_MIRA, 10, 14, SC_TOWN, 15, 10, 0},
    {SC_TOM, 10, 14, SC_TOWN, 25, 10, 0},
    {SC_LILY, 10, 14, SC_TOWN, 8, 23, 0},
};
#define DOOR_COUNT ((int)(sizeof(doors) / sizeof(doors[0])))

static const Sched sched_mira[] = {
    {0, SC_MIRA, 10, 5}, {540, SC_TOWN, 19, 15},
    {660, SC_SHOP, 16, 3}, {1020, SC_TOWN, 20, 16},
    {1200, SC_MIRA, 10, 5}, {1440, 0, 0, 0},
};
static const Sched sched_tom[] = {
    {0, SC_TOM, 10, 5}, {420, SC_TOWN, 19, 14},
    {720, SC_TOM, 10, 5}, {840, SC_TOWN, 14, 14},
    {1080, SC_TOM, 10, 5}, {1440, 0, 0, 0},
};
static const Sched sched_lily[] = {
    {0, SC_LILY, 10, 5}, {540, SC_TOWN, 16, 14},
    {720, SC_LILY, 10, 5}, {840, SC_TOWN, 22, 18},
    {1080, SC_LILY, 10, 5}, {1440, 0, 0, 0},
};

static const NpcDef npc_def[3] = {
    {"mira", SC_MIRA, 9,
     {{"hello!", "seeds today?"},
      {"you farm well.", "the shop is open."},
      {"you are my", "best friend."}},
     {"spring seeds are in.", "summer is warm.", "pumpkins soon!", "stay warm."},
     {"i am mira.", "i run the shop."},
     {"rain today!", "no watering."},
     {"snow is pretty.", "stay warm."}},
    {"tom", SC_TOM, 2,
     {{"nice day.", "water first."},
      {"your soil", "looks happy."},
      {"i will help", "you anytime."}},
     {"rain helps crops.", "water at dawn.", "harvest time.", "rest your back."},
     {"i am tom.", "i farm too."},
     {"free water.", "good day."},
     {"cold day.", "rest inside."}},
    {"lily", SC_LILY, 4,
     {{"your farm", "is pretty!"},
      {"i found a bug.", "see you!"},
      {"lets play", "together!"}},
     {"flowers bloom!", "bugs are out.", "leaves fall.", "snow is fun."},
     {"i am lily.", "lets be friends."},
     {"puddles!", "stay dry."},
     {"snow!", "cold day."}},
};


typedef struct {
    const char *title;
    uint8_t kind;      /* 0 crop, 1 fish, 2 shell */
    uint8_t id;
    uint8_t count;
    uint16_t gold;
    uint8_t seed_id;
    uint8_t seed_count;
} Quest;

typedef struct {
    const char *start[4];
    const char *ask[2];
    const char *done[3];
} StoryLines;

static const Quest quests[3][2] = {
    { {"seed shelf", 0, 1, 3, 120, 0, 2}, {"pumpkin", 0, 8, 1, 300, 2, 2} },
    { {"pier lunch", 1, 0, 2, 120, 1, 2}, {"big fish", 1, 1, 1, 300, 4, 2} },
    { {"flower crown", 0, 3, 2, 120, 0, 2}, {"shell crown", 2, 0, 3, 300, 2, 2} },
};

static const StoryLines story_lines[3][2] = {
    {
        {{"my grandma", "had this shop.", "the shelf looks", "empty today."},
         {"bring 3 potatoes?", "for the shelf."},
         {"the shelf is full.", "thank you!", "take this!"}},
        {{"grandma grew", "big pumpkins.", "one would make", "the shop glow."},
         {"bring 1 pumpkin?", "please?"},
         {"it glows again.", "like her shop.", "take this!"}}
    },
    {
        {{"this pier is old.", "i fished here", "with my dad.", "help me fix it?"},
         {"bring 2 sardines?", "lunch first."},
         {"planks are ready.", "thank you.", "take this!"}},
        {{"the last plank", "needs a big fish.", "bring 1 bass?", "then we finish."},
         {"bring 1 bass?", "one big one."},
         {"dad would like you.", "sit here anytime.", "take this!"}}
    },
    {
        {{"i want to make", "a flower crown.", "for my mom.", "bring 2 berries?"},
         {"2 berries, please.", "for the crown."},
         {"it is pretty!", "mom will smile.", "take this!"}},
        {{"the crown needs", "3 shells.", "from the beach.", "can you find them?"},
         {"3 shells, please.", "from the beach."},
         {"you are my", "best friend.", "take this!"}}
    },
};

static Actor npcs[3];
static int npc_points[3];
static int npc_talked[3];
static int npc_gifted[3];

static uint8_t farm_tiles[FARM_W * FARM_H];
static uint8_t town_tiles[FARM_W * FARM_H];
static uint8_t home_tiles[INT_W * INT_H];
static uint8_t shop_tiles[INT_W * INT_H];
static uint8_t mira_tiles[INT_W * INT_H];
static uint8_t tom_tiles[INT_W * INT_H];
static uint8_t lily_tiles[INT_W * INT_H];
static uint8_t beach_tiles[FARM_W * FARM_H];
static uint8_t mine_tiles[INT_W * INT_H];

static uint8_t *scene_tiles[SC_COUNT] = {
    farm_tiles, town_tiles, home_tiles, shop_tiles,
    mira_tiles, tom_tiles, lily_tiles, beach_tiles, mine_tiles,
};
static const int scene_w[SC_COUNT] = {FARM_W, FARM_W, INT_W, INT_W, INT_W, INT_W, INT_W, FARM_W, INT_W};
static const int scene_h[SC_COUNT] = {FARM_H, FARM_H, INT_H, INT_H, INT_H, INT_H, INT_H, FARM_H, INT_H};
static const int scene_indoor[SC_COUNT] = {0, 0, 1, 1, 1, 1, 1, 0, 1};

static int scene_id;

static uint16_t tile_img[T_COUNT][TILE * TILE];

static uint8_t plot_crop[PLOTS];
static uint8_t plot_growth[PLOTS];
static uint8_t plot_water[PLOTS];

static int player_x, player_y, facing;
static int player_walk, player_walk_tick;
static int state;
static int boot_loaded;
static int title_t;
#define TITLE_END 180
static int season, day, year, minutes;
static int coins, energy, bin_value, water_can;
static int house_level, can_level;
static int energy_max = 30, water_can_max = 15;
static int status_upg_sel;
static int rod_owned;
static uint8_t shell_mask;
static uint8_t fish[4];
static uint8_t bin_fish[4];
static uint8_t summary_fish[4];
static int bag_sel;
enum { IT_NONE = 0, IT_HOE, IT_CAN, IT_ROD, IT_SWORD, IT_SEED, IT_CROP, IT_FISH };
static int sel_kind = IT_HOE, sel_id;
static int pet_owned;                 /* bit 0 cat, bit 1 chicken */
static uint8_t pet_aff[2];
static uint8_t pet_fed;               /* bit 0 cat, bit 1 chicken */
static int pet_x[2], pet_y[2], pet_dir[2], pet_frame[2], pet_wait[2];
static int dino_unlocked;
static int dino_x, dino_y, dino_frame, dino_wait;
static uint8_t secrets;
#define SEC_CAMPFIRE 1
#define SEC_LETTER   2
#define SEC_GOLDEN   4
#define SEC_GOLD_TOLD 8
#define SEC_FOSSIL   16
static int wish_done, wish_rain, dolphin_done, dolphin_timer, cat_bed_block;
static int pet_shop_open, pet_shop_sel;
static uint8_t story[3];
static uint8_t shell_count;
static uint8_t ore_count;
static int player_hp = 3;
static int slime_x[3], slime_y[3], slime_hp[3], slime_alive[3];
static int slime_hit_cd;
static const char *pet_name[3] = {"none", "cat", "chicken"};
static const int pet_price[3] = {0, 300, 200};
enum { FISH_IDLE, FISH_WAIT, FISH_BITE, FISH_GAME };
static int fish_state, fish_timer, fish_type, fish_pos, fish_vel, fish_green, fish_catch, fish_zone_h;
static char fish_msg[24];
static const char *fish_name[4] = {"sardine", "bass", "trout", "tuna"};
static const uint8_t fish_price[4] = {30, 50, 70, 100};
static const uint8_t shell_x[4] = {10, 14, 8, 18};
static const uint8_t shell_y[4] = {18, 22, 24, 10};

static const int house_cost[5] = {300, 1000, 2500, 5000, 0};
static const int can_cost[3] = {200, 600, 0};
static const int can_cap[3] = {15, 25, 40};
static uint8_t bin_items[16];
static uint8_t seeds[16], produce[16];
static int weather;
static int shop_open, shop_sel, shop_qty;
static int bin_open, summary_open;
static int menu_open, menu_page, menu_sel;
static uint8_t summary_items[16];
static int summary_gold;
static int status_open, status_page;
static int dlg_open, dlg_npc, dlg_page, dlg_pages;
static const char *dlg_lines[4];
static char gift_buf[24];
static int fade_timer, fade_scene, fade_x, fade_y, fade_dir;
static int sleep_timer;
static char sleep_text[16];
static unsigned time_ticks;
static unsigned water_anim;
static int view_num = 1, view_den = 1;
#define ZOOM_NUM 5
#define ZOOM_DEN 4

static const char *message;
static int message_timer;
static char msg_buf[24];

static int a_prev, b_prev, menu_prev, up_prev, down_prev, left_prev, right_prev;
static int a_hold;
static int ending;

int game_clock_mhz = 96;

extern const uint8_t font8[576];

static int can_stand(int px, int py);
static void game_to_save(SaveData *d);
static void sleep_night(void);
static int rnd(void);
static int solid(uint8_t t);
static void target_tile(int *tx, int *ty);
static void save_now(void);
static void pet_update(void);
static void mine_update(void);
static void draw_item_icon(uint16_t *fb, int x, int y, int id, int seed);
static void draw_fish_icon(uint16_t *fb, int x, int y, int type);
static void draw_tool_icon(uint16_t *fb, int x, int y, int kind);
static char story_marker(int i);
static void story_desc(int i, char *out);
static int iabs(int v);

static int clamp(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline uint32_t hash2(int x, int y)
{
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u;
    h ^= h >> 13;
    return h;
}

static inline void put_px(uint16_t *fb, int x, int y, uint16_t c)
{
    if (view_num == view_den) {
        if ((unsigned)x < GAME_W && (unsigned)y < GAME_H) {
            fb[x * GAME_H + y] = c;
        }
        return;
    }
    int x0 = x * view_num / view_den;
    int x1 = (x + 1) * view_num / view_den;
    int y0 = y * view_num / view_den;
    int y1 = (y + 1) * view_num / view_den;
    if (x1 <= x0) x1 = x0 + 1;
    if (y1 <= y0) y1 = y0 + 1;
    for (int i = x0; i < x1; i++) {
        if ((unsigned)i >= GAME_W) continue;
        for (int j = y0; j < y1; j++) {
            if ((unsigned)j < GAME_H) fb[i * GAME_H + j] = c;
        }
    }
}

static void fill_rect(uint16_t *fb, int x, int y, int w, int h, uint16_t c)
{
    if (view_num == view_den) {
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                put_px(fb, x + i, y + j, c);
            }
        }
        return;
    }
    int x0 = x * view_num / view_den;
    int x1 = (x + w) * view_num / view_den;
    int y0 = y * view_num / view_den;
    int y1 = (y + h) * view_num / view_den;
    for (int i = x0; i < x1; i++) {
        if ((unsigned)i >= GAME_W) continue;
        for (int j = y0; j < y1; j++) {
            if ((unsigned)j < GAME_H) fb[i * GAME_H + j] = c;
        }
    }
}

static void draw_char(uint16_t *fb, int x, int y, char ch, uint16_t c)
{
    if (ch < 32 || ch > 127) ch = '?';
    const uint8_t *g = &font8[(ch - 32) * 6];
    for (int i = 0; i < 6; i++) {
        uint8_t col = g[i];
        for (int j = 0; j < 8; j++) {
            if (col & (1 << j)) put_px(fb, x + i, y + j, c);
        }
    }
}

static void draw_text(uint16_t *fb, int x, int y, const char *s, uint16_t c)
{
    while (*s) {
        draw_char(fb, x, y, *s++, c);
        x += 6;
    }
}

static void draw_text_shadow(uint16_t *fb, int x, int y, const char *s, uint16_t c)
{
    draw_text(fb, x + 1, y + 1, s, C_BLACK);
    draw_text(fb, x, y, s, c);
}

static void draw_text_big(uint16_t *fb, int x, int y, const char *s, uint16_t c)
{
    while (*s) {
        char ch = *s++;
        if (ch < 32 || ch > 127) ch = '?';
        const uint8_t *g = &font8[(ch - 32) * 6];
        for (int i = 0; i < 6; i++) {
            uint8_t col = g[i];
            for (int j = 0; j < 8; j++) {
                if (col & (1 << j)) {
                    fill_rect(fb, x + i * 2, y + j * 2, 2, 2, c);
                }
            }
        }
        x += 12;
    }
}

static void draw_text_big_shadow(uint16_t *fb, int x, int y, const char *s, uint16_t c)
{
    draw_text_big(fb, x + 2, y + 2, s, C_BLACK);
    draw_text_big(fb, x, y, s, c);
}

static void draw_num_big(uint16_t *fb, int x, int y, int v, uint16_t c)
{
    char b[12];
    int n = 0;
    if (v <= 0) {
        b[n++] = '0';
    } else {
        while (v > 0) {
            b[n++] = (char)('0' + v % 10);
            v /= 10;
        }
    }
    char out[12];
    int k = 0;
    while (n > 0) out[k++] = b[--n];
    out[k] = 0;
    draw_text_big(fb, x, y, out, c);
}

static void draw_num_big_shadow(uint16_t *fb, int x, int y, int v, uint16_t c)
{
    draw_num_big(fb, x + 2, y + 2, v, C_BLACK);
    draw_num_big(fb, x, y, v, c);
}

static void draw_gold(uint16_t *fb, int x, int y, int v)
{
    int n = 1, t = v;
    while (t >= 10) { n++; t /= 10; }
    draw_num_big_shadow(fb, x, y, v, C_GOLD);
    draw_text_big_shadow(fb, x + n * 12 + 4, y, "g", C_GOLD);
}

static void draw_time_big(uint16_t *fb, int x, int y, int mins)
{
    int h = mins / 60;
    int m = mins % 60;
    char d[6];
    d[0] = (char)('0' + h / 10);
    d[1] = (char)('0' + h % 10);
    d[2] = ':';
    d[3] = (char)('0' + m / 10);
    d[4] = (char)('0' + m % 10);
    d[5] = 0;
    draw_text_big_shadow(fb, x, y, d, C_WHITE);
}

/* --- palette / tiles --------------------------------------------------- */

static void palette_set(int s)
{
    if (s == 0) {
        C_GRASS = RGB(76, 126, 64);  C_GRASS_D = RGB(54, 94, 50);
        C_GRASS_L = RGB(108, 160, 82);
        C_SOIL = RGB(134, 92, 58);   C_SOIL_D = RGB(96, 64, 42);
        C_WATER = RGB(54, 126, 146); C_WATER_D = RGB(32, 82, 106);
        C_WATER_L = RGB(112, 182, 190);
        C_PATH = RGB(196, 166, 112); C_PATH_D = RGB(146, 116, 78);
        C_LEAF = RGB(46, 100, 46);   C_LEAF_L = RGB(82, 138, 62);
        C_LEAF_D = RGB(30, 68, 34);
    } else if (s == 1) {
        C_GRASS = RGB(62, 134, 60);  C_GRASS_D = RGB(42, 96, 46);
        C_GRASS_L = RGB(98, 170, 76);
        C_SOIL = RGB(138, 96, 58);   C_SOIL_D = RGB(98, 66, 42);
        C_WATER = RGB(42, 136, 158); C_WATER_D = RGB(26, 90, 114);
        C_WATER_L = RGB(104, 196, 206);
        C_PATH = RGB(202, 172, 116); C_PATH_D = RGB(150, 120, 80);
        C_LEAF = RGB(38, 108, 44);   C_LEAF_L = RGB(76, 154, 62);
        C_LEAF_D = RGB(24, 72, 34);
    } else if (s == 2) {
        C_GRASS = RGB(138, 126, 64); C_GRASS_D = RGB(102, 92, 48);
        C_GRASS_L = RGB(174, 154, 82);
        C_SOIL = RGB(138, 96, 58);   C_SOIL_D = RGB(98, 66, 42);
        C_WATER = RGB(58, 116, 132); C_WATER_D = RGB(36, 78, 96);
        C_WATER_L = RGB(122, 174, 178);
        C_PATH = RGB(188, 156, 108); C_PATH_D = RGB(140, 110, 74);
        C_LEAF = RGB(160, 98, 42);   C_LEAF_L = RGB(202, 140, 56);
        C_LEAF_D = RGB(110, 64, 32);
    } else {
        C_GRASS = RGB(204, 214, 224); C_GRASS_D = RGB(166, 180, 196);
        C_GRASS_L = RGB(236, 242, 248);
        C_SOIL = RGB(118, 102, 90);   C_SOIL_D = RGB(92, 78, 68);
        C_WATER = RGB(154, 204, 224); C_WATER_D = RGB(110, 160, 190);
        C_WATER_L = RGB(214, 238, 248);
        C_PATH = RGB(188, 188, 198);  C_PATH_D = RGB(148, 148, 162);
        C_LEAF = RGB(130, 116, 102);  C_LEAF_L = RGB(168, 154, 138);
        C_LEAF_D = RGB(96, 86, 78);
    }
}

static void tiles_build(void)
{
    for (int t = 0; t < T_COUNT; t++) {
        for (int y = 0; y < TILE; y++) {
            for (int x = 0; x < TILE; x++) {
                uint16_t c = C_GRASS;
                uint32_t h = hash2(x, y);

                switch (t) {
                case T_GRASS:
                    c = C_GRASS;
                    if ((h & 15) == 0) c = C_GRASS_D;
                    if ((h & 63) == 0) c = C_GRASS_L;
                    if (((x % 9) == 2 && (y % 7) == 3) || ((x % 9) == 3 && (y % 7) == 3)) c = C_GRASS_L;
                    if ((x % 9) == 2 && (y % 7) == 4) c = C_GRASS_D;
                    if ((h & 255) == 0) c = C_ROSE;
                    break;
                case T_GRASS2:
                    c = C_GRASS;
                    if ((h & 15) == 0) c = C_GRASS_D;
                    if ((h & 63) == 0) c = C_GRASS_L;
                    if (x >= 6 && x <= 9 && y >= 6 && y <= 9) c = C_ROSE;
                    if (x >= 7 && x <= 8 && y >= 7 && y <= 8) c = C_GOLD;
                    if (x >= 7 && x <= 8 && y >= 10 && y <= 13) c = C_LEAF;
                    if (x == 7 && y == 10) c = C_LEAF_L;
                    break;
                case T_SOIL:
                    c = C_SOIL;
                    if ((y % 5) == 0) c = C_SOIL_D;
                    if ((y % 5) == 1) c = RGB(154, 108, 68);
                    if ((h & 31) == 0) c = C_SOIL_D;
                    if ((x % 8 == 2) && (y % 5 == 3)) c = RGB(166, 118, 74);
                    break;
                case T_SOIL_W:
                    c = RGB(82, 58, 42);
                    if ((y % 5) == 0) c = RGB(58, 40, 30);
                    if ((y % 5) == 1) c = RGB(104, 74, 52);
                    if ((h & 31) == 0) c = RGB(62, 44, 32);
                    if (x >= 11 && x <= 13 && y >= 3 && y <= 5) c = C_WATER_L;
                    break;
                case T_WATER:
                    c = C_WATER;
                    if (y == 0) c = C_WATER_D;
                    if ((y == 3 || y == 4) && (x % 8) < 5) c = C_WATER_L;
                    if ((y == 9 || y == 10) && (x % 8) > 2) c = C_WATER_L;
                    if ((y == 4 || y == 10) && (x % 16) == 7) c = RGB(226, 244, 240);
                    if ((y == 5 || y == 11) && (x % 16) == 8) c = C_WATER_L;
                    break;
                case T_PATH:
                    c = C_PATH;
                    if ((x % 5) == 0 || (y % 5) == 0) c = C_PATH_D;
                    else if ((hash2(x / 5, y / 5) & 3) == 0) c = RGB(212, 182, 128);
                    if ((h & 31) == 0) c = C_PATH_D;
                    break;
                case T_TREE: {
                    int dx = x - 8, dy = y - 7;
                    int d2 = dx * dx + dy * dy;
                    c = C_GRASS;
                    if (d2 < 86) c = C_LEAF_D;
                    if (d2 < 68) c = C_LEAF;
                    if (d2 < 46) c = C_LEAF_L;
                    if (d2 < 30 && dx < 2 && dy < 2) c = RGB(108, 170, 82);
                    if (d2 < 12) c = C_LEAF;
                    if (x >= 7 && x <= 8 && y >= 11) c = C_WOOD;
                    if (x == 7 && y >= 11) c = RGB(160, 112, 62);
                    if (y >= 14 && x >= 4 && x <= 11) c = C_GRASS_D;
                    if (y == 15 && x >= 5 && x <= 10) c = C_GRASS_D;
                    break;
                }
                case T_WALL:
                    c = C_WOOD;
                    if ((y % 5) == 0) c = C_SOIL_D;
                    if ((y % 5) == 1) c = C_WOOD_L;
                    if (x == 15) c = C_SOIL_D;
                    if ((h & 63) == 0) c = RGB(128, 90, 54);
                    break;
                case T_WINDOW:
                    c = C_WOOD;
                    if (x >= 2 && x <= 13 && y >= 2 && y <= 12) {
                        c = RGB(86, 128, 138);
                        if (x <= 3 || x >= 12 || y <= 3) c = RGB(54, 92, 108);
                        if (x >= 4 && x <= 5 && y >= 4 && y <= 6) c = RGB(176, 216, 216);
                        if (x >= 8 && x <= 9 && y >= 8 && y <= 9) c = RGB(144, 190, 196);
                        if (x == 2 || x == 13 || y == 2 || y == 12 || x == 7 || x == 8 || y == 7) c = C_WOOD;
                        if (x == 3 && y >= 4 && y <= 10) c = RGB(198, 72, 66);
                        if (x == 12 && y >= 4 && y <= 10) c = RGB(198, 72, 66);
                    }
                    if (y >= 13 && x >= 3 && x <= 12) c = C_WOOD_L;
                    break;
                case T_ROOF:
                    c = C_ROOF;
                    if ((y % 4) == 0) c = C_ROOF_D;
                    if ((y % 4) == 1) c = RGB(176, 82, 56);
                    if (((x + (y / 4) * 5) % 10) == 0) c = C_ROOF_D;
                    if (y == 15) c = RGB(74, 38, 30);
                    break;
                case T_DOOR:
                    c = C_WOOD;
                    if (x <= 2 || x >= 13 || y <= 1 || y >= 15) c = C_SOIL_D;
                    if (x >= 4 && x <= 11 && y >= 3 && y <= 7) c = C_WOOD_L;
                    if (x >= 4 && x <= 11 && y >= 9 && y <= 12) c = C_WOOD_L;
                    if (x == 7 && y >= 3 && y <= 12) c = C_SOIL_D;
                    if (x == 11 && y == 8) c = C_GOLD;
                    break;
                case T_FLOWER:
                    c = C_GRASS;
                    if ((h & 7) == 0) c = C_GRASS_D;
                    if (x >= 5 && x <= 10 && y >= 5 && y <= 10) c = C_ROSE;
                    if (x >= 7 && x <= 8 && y >= 7 && y <= 8) c = C_GOLD;
                    if (x >= 7 && x <= 8 && y >= 11 && y <= 13) c = C_LEAF;
                    break;
                case T_FENCE:
                    c = C_GRASS;
                    if (y >= 3 && y <= 5) c = C_FENCE_L;
                    if (y == 6) c = C_FENCE_D;
                    if (y >= 10 && y <= 12) c = C_FENCE_L;
                    if (y == 13) c = C_FENCE_D;
                    if (x >= 7 && x <= 9 && y >= 2 && y <= 13) c = C_FENCE;
                    if (x == 7 && y >= 2 && y <= 13) c = C_FENCE_L;
                    break;
                case T_GATE:
                    c = C_GRASS;
                    if (x <= 2) { c = C_FENCE; if (x == 1) c = C_FENCE_L; }
                    if (x >= 13) { c = C_FENCE; if (x == 14) c = C_FENCE_L; }
                    if (y >= 3 && y <= 5 && (x <= 2 || x >= 13)) c = C_FENCE_L;
                    if (y >= 10 && y <= 12 && (x <= 2 || x >= 13)) c = C_FENCE_L;
                    break;
                case T_SAND:
                    c = C_SAND;
                    if ((h & 15) == 0) c = C_SAND_D;
                    if ((y % 6) == 0) c = RGB(234, 214, 168);
                    if ((y % 6) == 1 && (x % 7) < 4) c = RGB(212, 188, 140);
                    if ((h & 63) == 0) c = RGB(246, 226, 180);
                    break;
                case T_CHIMNEY:
                    c = C_ROOF;
                    if (y == 0 || (y % 5) == 0) c = C_ROOF_D;
                    if (x >= 5 && x <= 10 && y <= 7) {
                        c = C_BRICK;
                        if ((y % 3) == 0) c = C_BRICK_D;
                    }
                    break;
                case T_STAND:
                    c = C_GRASS;
                    if (y >= 6) c = C_WOOD;
                    if (y >= 7 && y <= 12) c = C_WOOD_L;
                    if (x <= 1 || x >= 14) c = C_WOOD;
                    if (y >= 3 && y <= 8 && x >= 4 && x <= 11) c = C_LEAF;
                    if (y >= 4 && y <= 6 && x >= 5 && x <= 10) c = C_GOLD;
                    break;
                case T_BIN:
                    c = C_GRASS;
                    if (x >= 2 && x <= 13 && y >= 3 && y <= 13) c = C_WOOD;
                    if (x >= 3 && x <= 12 && y >= 4 && y <= 11) c = C_WOOD_L;
                    if (y == 4 || y == 12) c = C_SOIL_D;
                    if (x >= 6 && x <= 9 && y >= 7 && y <= 9) c = C_GOLD;
                    break;
                case T_LAMP:
                    c = C_GRASS;
                    if (x >= 7 && x <= 8 && y >= 4) c = C_WOOD;
                    if (y >= 1 && y <= 4 && x >= 5 && x <= 10) c = C_GOLD;
                    break;
                case T_FOUNTAIN:
                    c = C_WATER;
                    if (x <= 1 || y <= 1 || x >= 14 || y >= 14) c = C_SAND_D;
                    if ((y % 5) == 2) c = C_WATER_L;
                    break;
                case T_FLOOR:
                    c = C_FLOOR;
                    if ((y % 7) == 0) c = C_FLOOR_D;
                    if ((y % 7) == 1) c = RGB(168, 128, 84);
                    if ((h & 31) == 0) c = RGB(160, 120, 80);
                    if (x == 15) c = C_FLOOR_D;
                    break;
                case T_WALL_IN:
                    c = C_WALL_IN;
                    if ((x % 6) == 0) c = C_WALL_IN_D;
                    if ((x % 6) == 1) c = RGB(212, 192, 160);
                    if (y >= 14) c = C_FLOOR_D;
                    if (y == 13) c = C_WALL_IN_D;
                    if (y == 12) c = RGB(210, 188, 152);
                    break;
                case T_BED:
                    c = RGB(230, 226, 214);
                    if (x <= 2 || x >= 13) c = C_WOOD;
                    if (y <= 2) c = C_WOOD;
                    if (y == 1) c = RGB(156, 112, 66);
                    if (x >= 3 && x <= 12 && y >= 4 && y <= 9) c = RGB(250, 246, 230);
                    if (y == 5 || y == 8) c = RGB(212, 206, 192);
                    if (x <= 4 || x >= 11) c = RGB(198, 190, 174);
                    if (y >= 12) c = RGB(184, 66, 68);
                    if (y == 13) c = RGB(212, 92, 90);
                    if (y >= 15) c = C_SOIL_D;
                    break;
                case T_BED2:
                    c = RGB(184, 66, 68);
                    if (x <= 1 || x >= 14) c = C_WOOD;
                    if (y <= 3) c = RGB(242, 236, 220);
                    if (y == 3) c = RGB(210, 202, 186);
                    if (y >= 4 && y <= 13 && (x % 5) == 0) c = RGB(150, 50, 54);
                    if (y >= 4 && y <= 13 && (x % 5) == 1) c = RGB(202, 82, 82);
                    if (y >= 14) c = C_WOOD;
                    if (y == 15) c = RGB(76, 48, 30);
                    break;
                case T_TABLE:
                    c = C_WOOD_L;
                    if (y <= 2) c = RGB(190, 150, 96);
                    if (x <= 1 || x >= 14 || y >= 13) c = C_WOOD;
                    break;
                case T_COUNTER:
                    c = C_WOOD;
                    if (y <= 5) c = C_WOOD_L;
                    if (y == 6) c = C_SOIL_D;
                    break;
                case T_SHELF:
                    c = C_WOOD;
                    if (y % 6 < 2) c = C_WOOD_L;
                    if (x % 6 == 2) c = C_GOLD;
                    break;
                case T_RUG:
                    c = RGB(178, 62, 62);
                    if (x <= 1 || y <= 1 || x >= 14 || y >= 14) c = RGB(230, 190, 90);
                    break;
                case T_PLANT:
                    c = C_FLOOR;
                    if (x >= 4 && x <= 11 && y >= 4 && y <= 11) c = C_LEAF;
                    if (x >= 6 && x <= 9 && y >= 6 && y <= 9) c = C_LEAF_L;
                    if (y >= 12 && x >= 6 && x <= 9) c = RGB(150, 90, 60);
                    break;
                case T_STOVE:
                    c = RGB(92, 94, 98);
                    if (y <= 3) c = RGB(140, 142, 146);
                    if (x >= 4 && x <= 11 && y >= 6 && y <= 10) c = RGB(40, 40, 44);
                    break;
                case T_DOOR_IN:
                    c = C_WOOD;
                    if (x <= 2 || x >= 13 || y <= 1) c = C_SOIL_D;
                    if (x == 11 && y == 9) c = C_GOLD;
                    break;
                case T_WINDOW_IN:
                    c = C_WALL_IN;
                    if (x >= 2 && x <= 13 && y >= 2 && y <= 12) {
                        c = C_WATER_L;
                        if (x == 2 || x == 13 || y == 2 || y == 12 || x == 7 || y == 7) c = C_WOOD;
                    }
                    break;
                case T_CHAIR:
                    c = C_FLOOR;
                    if (y >= 5 && y <= 12 && x >= 4 && x <= 11) c = C_WOOD_L;
                    if (y >= 12 && x >= 4 && x <= 11) c = C_WOOD;
                    if (y >= 1 && y <= 4 && x >= 4 && x <= 11) c = C_WOOD;
                    break;
                case T_BOOKSHELF:
                    c = C_WOOD;
                    if (y % 6 < 2) c = C_WOOD_L;
                    if (x >= 2 && x <= 13) {
                        if ((y > 2 && y < 6) || (y > 8 && y < 12)) c = RGB(120, 60, 50);
                    }
                    break;
                case T_CRATE:
                    c = C_WOOD;
                    if (x <= 1 || y <= 1 || x >= 14 || y >= 14) c = C_FLOOR_D;
                    if (x >= 3 && x <= 12 && y >= 3 && y <= 12) c = C_WOOD_L;
                    break;
                case T_AWNING:
                    c = (x % 8 < 4) ? RGB(210, 70, 70) : C_WHITE;
                    if (y >= 13) c = C_ROOF_D;
                    break;
                case T_SIGN:
                    c = C_WOOD;
                    if (y >= 2 && y <= 12 && x >= 1 && x <= 14) c = C_WOOD_L;
                    if (y >= 4 && y <= 10 && x >= 4 && x <= 11) c = C_GOLD;
                    if (y == 11 && x >= 6 && x <= 9) c = C_WOOD;
                    break;
                case T_SCARECROW:
                    c = C_GRASS;
                    if (x >= 7 && x <= 8 && y >= 4) c = C_WOOD;
                    if (x >= 4 && x <= 11 && y >= 6 && y <= 12) c = C_WOOD_L;
                    if (y >= 1 && y <= 4 && x >= 4 && x <= 11) c = C_GOLD;
                    if (y >= 5 && y <= 8 && x >= 5 && x <= 10) c = RGB(190, 150, 90);
                    if (x == 6 && y == 6) c = C_BLACK;
                    if (x == 9 && y == 6) c = C_BLACK;
                    break;
                case T_BARREL:
                    c = C_GRASS;
                    if (x >= 2 && x <= 13 && y >= 3 && y <= 13) c = C_WOOD;
                    if (y == 4 || y == 8 || y == 12) c = C_SOIL_D;
                    if (x >= 4 && x <= 11 && y >= 5 && y <= 11) c = C_WOOD_L;
                    break;
                case T_MAILBOX:
                    c = C_GRASS;
                    if (x >= 7 && x <= 8 && y >= 8) c = C_WOOD;
                    if (x >= 4 && x <= 11 && y >= 3 && y <= 8) c = RGB(178, 62, 62);
                    if (y == 5 && x >= 5 && x <= 10) c = C_WHITE;
                    break;
                case T_BENCH:
                    c = C_GRASS;
                    if (x >= 1 && x <= 14 && y >= 6 && y <= 9) c = C_WOOD_L;
                    if (y >= 10 && y <= 13 && (x <= 3 || x >= 12)) c = C_WOOD;
                    break;
                case T_PIER:
                    c = C_WATER;
                    if (y < 3 || y > 12) c = C_WOOD;
                    else {
                        c = C_WOOD_L;
                        if ((y % 4) == 0) c = C_WOOD;
                        if (y == 4) c = RGB(186, 138, 86);
                        if (x == 15) c = C_WOOD;
                    }
                    if (y == 15) c = C_WOOD;
                    break;
                case T_ROCK:
                    c = C_SAND;
                    if (x >= 3 && x <= 12 && y >= 4 && y <= 11) c = RGB(120, 120, 120);
                    if (x >= 4 && x <= 10 && y >= 3 && y <= 8) c = RGB(170, 170, 170);
                    if (x >= 5 && x <= 8 && y >= 4 && y <= 6) c = RGB(210, 210, 210);
                    break;
                case T_PAINTING:
                    c = C_WALL_IN;
                    if (x <= 1 || x >= 14 || y <= 1 || y >= 14) c = C_GOLD;
                    if (x >= 3 && x <= 12 && y >= 3 && y <= 12) c = RGB(102, 156, 176);
                    if (x >= 3 && x <= 12 && y >= 9 && y <= 12) c = RGB(92, 146, 84);
                    if (x >= 5 && x <= 7 && y >= 5 && y <= 8) c = C_WHITE;
                    if (x >= 9 && x <= 12 && y >= 3 && y <= 7) c = C_WATER_L;
                    break;
                case T_CLOCK:
                    c = C_WALL_IN;
                    if (x >= 1 && x <= 14 && y >= 1 && y <= 14) c = C_WOOD;
                    if (x >= 3 && x <= 12 && y >= 3 && y <= 12) c = C_WHITE;
                    if (x >= 7 && x <= 8 && y >= 5 && y <= 9) c = RGB(40, 36, 30);
                    if (x >= 7 && x <= 10 && y >= 7 && y <= 8) c = RGB(40, 36, 30);
                    break;
                case T_ROOF2:
                    c = RGB(74, 96, 118);
                    if ((y % 4) == 0) c = RGB(46, 62, 82);
                    if ((y % 4) == 1) c = RGB(104, 130, 150);
                    if (((x + (y / 4) * 5) % 10) == 0) c = RGB(46, 62, 82);
                    if (y == 15) c = RGB(30, 42, 58);
                    break;
                case T_ROOF3:
                    c = RGB(126, 82, 62);
                    if ((y % 4) == 0) c = RGB(88, 54, 42);
                    if ((y % 4) == 1) c = RGB(158, 108, 78);
                    if (((x + (y / 4) * 5) % 10) == 0) c = RGB(88, 54, 42);
                    if (y == 15) c = RGB(58, 36, 30);
                    break;
                case T_WALL2:
                    c = RGB(150, 146, 136);
                    if ((y % 6) == 0) c = RGB(112, 108, 100);
                    if ((x % 8) == 0) c = RGB(112, 108, 100);
                    if ((y % 6) == 1) c = RGB(176, 170, 158);
                    if ((h & 63) == 0) c = RGB(128, 124, 116);
                    break;
                case T_FLOOR2:
                    c = RGB(102, 76, 54);
                    if ((y % 7) == 0) c = RGB(76, 54, 38);
                    if ((y % 7) == 1) c = RGB(122, 92, 64);
                    if (x == 15) c = RGB(76, 54, 38);
                    break;
                case T_WALL_IN2:
                    c = RGB(126, 148, 168);
                    if ((x % 6) == 0) c = RGB(94, 114, 136);
                    if ((x % 6) == 1) c = RGB(150, 170, 188);
                    if (y >= 14) c = C_FLOOR_D;
                    if (y == 13) c = RGB(94, 114, 136);
                    if (y == 12) c = RGB(160, 180, 196);
                    break;
                case T_BOWL:
                    c = C_FLOOR;
                    if (x >= 3 && x <= 12 && y >= 6 && y <= 12) c = RGB(150, 70, 60);
                    if (x >= 4 && x <= 11 && y >= 6 && y <= 9) c = RGB(190, 96, 78);
                    if (x >= 5 && x <= 10 && y >= 7 && y <= 8) c = RGB(110, 70, 44);
                    break;
                case T_BOWL2:
                    c = C_FLOOR;
                    if (x >= 3 && x <= 12 && y >= 5 && y <= 12) c = RGB(128, 92, 54);
                    if (x >= 4 && x <= 11 && y >= 6 && y <= 11) c = RGB(176, 128, 72);
                    if (x >= 5 && x <= 10 && y >= 7 && y <= 9) c = RGB(218, 180, 92);
                    if (x >= 5 && x <= 10 && y >= 10 && y <= 11) c = RGB(100, 72, 44);
                    break;
                case T_MINE_FLOOR:
                    c = RGB(92, 88, 82);
                    if ((h & 15) == 0) c = RGB(72, 68, 64);
                    if ((h & 63) == 0) c = RGB(112, 106, 96);
                    if ((x % 9) == 4 && (y % 11) == 3) c = RGB(66, 62, 58);
                    break;
                case T_MINE_WALL:
                    c = RGB(52, 48, 46);
                    if ((y % 5) == 0) c = RGB(34, 32, 30);
                    if ((x % 7) == 0) c = RGB(34, 32, 30);
                    if ((h & 31) == 0) c = RGB(76, 70, 64);
                    break;
                case T_ORE:
                    c = RGB(70, 66, 62);
                    if (x >= 3 && x <= 12 && y >= 3 && y <= 12) c = RGB(104, 96, 84);
                    if ((x >= 5 && x <= 7 && y >= 5 && y <= 7) ||
                        (x >= 9 && x <= 11 && y >= 8 && y <= 10)) c = RGB(204, 116, 58);
                    if (x == 6 && y == 5) c = RGB(248, 190, 100);
                    if (x == 10 && y == 9) c = RGB(248, 190, 100);
                    break;
                case T_LADDER:
                    c = RGB(92, 88, 82);
                    if ((x == 4 || x == 5 || x == 10 || x == 11) && y >= 1) c = RGB(150, 104, 62);
                    if (y >= 4 && (y % 4) == 0 && x >= 5 && x <= 10) c = RGB(190, 146, 88);
                    break;
                default:
                    c = C_BLACK;
                    break;
                }
                tile_img[t][y * TILE + x] = c;
            }
        }
    }
}

/* --- maps -------------------------------------------------------------- */

static void fill_map(uint8_t *t, int w, int h, uint8_t v)
{
    for (int i = 0; i < w * h; i++) t[i] = v;
}

static void build_farm(void)
{
    for (int y = 0; y < FARM_H; y++) {
        for (int x = 0; x < FARM_W; x++) {
            farm_tiles[y * FARM_W + x] = ((hash2(x, y) & 31) == 0) ? T_GRASS2 : T_GRASS;
        }
    }
    for (int x = 0; x < FARM_W; x++) {
        farm_tiles[x] = T_TREE;
        farm_tiles[(FARM_H - 1) * FARM_W + x] = T_TREE;
    }
    for (int y = 0; y < FARM_H; y++) {
        farm_tiles[y * FARM_W] = T_TREE;
        farm_tiles[y * FARM_W + FARM_W - 1] = T_TREE;
    }

    /* House (upgrades change roof/wall/windows). */
    if (house_level == 0) {
        for (int y = 3; y <= 5; y++)
            for (int x = 3; x <= 8; x++) farm_tiles[y * FARM_W + x] = T_ROOF;
        farm_tiles[2 * FARM_W + 7] = T_CHIMNEY;
        for (int y = 6; y <= 7; y++)
            for (int x = 3; x <= 8; x++) farm_tiles[y * FARM_W + x] = T_WALL;
        farm_tiles[6 * FARM_W + 3] = T_WINDOW;
        farm_tiles[6 * FARM_W + 8] = T_WINDOW;
        farm_tiles[7 * FARM_W + 5] = T_DOOR;
        farm_tiles[7 * FARM_W + 6] = T_DOOR;
    } else if (house_level == 1) {
        for (int y = 2; y <= 5; y++)
            for (int x = 3; x <= 8; x++) farm_tiles[y * FARM_W + x] = T_ROOF;
        farm_tiles[1 * FARM_W + 7] = T_CHIMNEY;
        for (int y = 6; y <= 8; y++)
            for (int x = 3; x <= 8; x++) farm_tiles[y * FARM_W + x] = T_WALL;
        farm_tiles[6 * FARM_W + 3] = T_WINDOW;
        farm_tiles[6 * FARM_W + 8] = T_WINDOW;
        farm_tiles[7 * FARM_W + 5] = T_DOOR;
        farm_tiles[7 * FARM_W + 6] = T_DOOR;
        farm_tiles[8 * FARM_W + 3] = T_FLOWER;
        farm_tiles[8 * FARM_W + 8] = T_FLOWER;
    } else if (house_level == 2) {
        for (int y = 1; y <= 5; y++)
            for (int x = 2; x <= 9; x++) farm_tiles[y * FARM_W + x] = T_ROOF2;
        farm_tiles[0 * FARM_W + 7] = T_CHIMNEY;
        for (int y = 6; y <= 8; y++)
            for (int x = 2; x <= 9; x++) farm_tiles[y * FARM_W + x] = T_WALL;
        farm_tiles[3 * FARM_W + 4] = T_WINDOW;
        farm_tiles[3 * FARM_W + 9] = T_WINDOW;
        farm_tiles[6 * FARM_W + 3] = T_WINDOW;
        farm_tiles[7 * FARM_W + 8] = T_WINDOW;
        farm_tiles[7 * FARM_W + 5] = T_DOOR;
        farm_tiles[7 * FARM_W + 6] = T_DOOR;
        farm_tiles[8 * FARM_W + 2] = T_FLOWER;
        farm_tiles[8 * FARM_W + 9] = T_FLOWER;
    } else if (house_level == 3) {
        for (int y = 0; y <= 5; y++)
            for (int x = 1; x <= 9; x++) farm_tiles[y * FARM_W + x] = T_ROOF2;
        farm_tiles[0 * FARM_W + 7] = T_CHIMNEY;
        for (int y = 6; y <= 8; y++)
            for (int x = 1; x <= 9; x++) farm_tiles[y * FARM_W + x] = T_WALL2;
        farm_tiles[3 * FARM_W + 2] = T_WINDOW;
        farm_tiles[3 * FARM_W + 8] = T_WINDOW;
        farm_tiles[6 * FARM_W + 2] = T_WINDOW;
        farm_tiles[7 * FARM_W + 8] = T_WINDOW;
        farm_tiles[7 * FARM_W + 5] = T_DOOR;
        farm_tiles[7 * FARM_W + 6] = T_DOOR;
        farm_tiles[8 * FARM_W + 1] = T_FLOWER;
        farm_tiles[8 * FARM_W + 9] = T_FLOWER;
    } else {
        for (int y = 0; y <= 5; y++)
            for (int x = 1; x <= 9; x++) farm_tiles[y * FARM_W + x] = T_ROOF3;
        farm_tiles[0 * FARM_W + 7] = T_CHIMNEY;
        for (int y = 6; y <= 8; y++)
            for (int x = 1; x <= 9; x++) farm_tiles[y * FARM_W + x] = T_WALL2;
        farm_tiles[3 * FARM_W + 1] = T_WINDOW;
        farm_tiles[3 * FARM_W + 9] = T_WINDOW;
        farm_tiles[6 * FARM_W + 1] = T_WINDOW;
        farm_tiles[7 * FARM_W + 8] = T_WINDOW;
        farm_tiles[7 * FARM_W + 5] = T_DOOR;
        farm_tiles[7 * FARM_W + 6] = T_DOOR;
        farm_tiles[8 * FARM_W + 1] = T_FLOWER;
        farm_tiles[8 * FARM_W + 9] = T_FLOWER;
    }

    /* Path from door to farm gate. */
    for (int y = 8; y <= 9; y++) {
        farm_tiles[y * FARM_W + 5] = T_PATH;
        farm_tiles[y * FARM_W + 6] = T_PATH;
    }
    for (int x = 5; x <= 9; x++) farm_tiles[9 * FARM_W + x] = T_PATH;

    /* Seed stand + shipping bin. */
    farm_tiles[8 * FARM_W + 9] = T_STAND;
    farm_tiles[10 * FARM_W + 8] = T_BIN;

    /* Fenced plot. */
    for (int y = 7; y <= 12; y++) {
        farm_tiles[y * FARM_W + 10] = T_FENCE;
        farm_tiles[y * FARM_W + 17] = T_FENCE;
    }
    for (int x = 10; x <= 17; x++) {
        farm_tiles[7 * FARM_W + x] = T_FENCE;
        farm_tiles[12 * FARM_W + x] = T_FENCE;
    }
    farm_tiles[9 * FARM_W + 10] = T_GATE;
    for (int y = 8; y <= 11; y++)
        for (int x = 11; x <= 16; x++) farm_tiles[y * FARM_W + x] = T_SOIL;

    /* Pond. */
    for (int y = 4; y <= 9; y++)
        for (int x = 19; x <= 26; x++) farm_tiles[y * FARM_W + x] = T_SAND;
    for (int y = 5; y <= 8; y++)
        for (int x = 20; x <= 25; x++) farm_tiles[y * FARM_W + x] = T_WATER;

    /* Trees and flowers. */
    for (int y = 2; y <= 4; y++)
        for (int x = 18; x <= 20; x++) farm_tiles[y * FARM_W + x] = T_TREE;
    for (int y = 15; y <= 18; y++)
        for (int x = 2; x <= 4; x++) farm_tiles[y * FARM_W + x] = T_TREE;
    for (int y = 13; y <= 16; y++)
        for (int x = 27; x <= 30; x++) farm_tiles[y * FARM_W + x] = T_TREE;
    farm_tiles[6 * FARM_W + 10] = T_BARREL;
    farm_tiles[8 * FARM_W + 7] = T_MAILBOX;
    farm_tiles[9 * FARM_W + 18] = T_SCARECROW;
    farm_tiles[16 * FARM_W + 8] = T_FLOWER;
    farm_tiles[16 * FARM_W + 9] = T_FLOWER;
    farm_tiles[19 * FARM_W + 20] = T_FLOWER;

    /* Exit path to town. */
    for (int x = 18; x <= 38; x++) farm_tiles[14 * FARM_W + x] = T_PATH;
    farm_tiles[14 * FARM_W + 38] = T_DOOR;
}

static void build_town(void)
{
    for (int y = 0; y < FARM_H; y++)
        for (int x = 0; x < FARM_W; x++)
            town_tiles[y * FARM_W + x] = ((hash2(x + 7, y + 3) & 31) == 0) ? T_GRASS2 : T_GRASS;

    for (int x = 0; x < FARM_W; x++) {
        town_tiles[x] = T_TREE;
        town_tiles[(FARM_H - 1) * FARM_W + x] = T_TREE;
    }
    for (int y = 0; y < FARM_H; y++) {
        town_tiles[y * FARM_W] = T_TREE;
        town_tiles[y * FARM_W + FARM_W - 1] = T_TREE;
    }

    for (int x = 1; x <= 38; x++) town_tiles[14 * FARM_W + x] = T_PATH;
    for (int y = 8; y <= 26; y++) town_tiles[y * FARM_W + 20] = T_PATH;
    for (int y = 10; y <= 14; y++) {
        town_tiles[y * FARM_W + 7] = T_PATH;
        town_tiles[y * FARM_W + 15] = T_PATH;
        town_tiles[y * FARM_W + 25] = T_PATH;
    }
    for (int y = 15; y <= 22; y++) town_tiles[y * FARM_W + 8] = T_PATH;
    town_tiles[14 * FARM_W] = T_PATH;
    town_tiles[14 * FARM_W + 39] = T_PATH;

    /* Beach path south. */
    for (int y = 27; y <= 28; y++) town_tiles[y * FARM_W + 20] = T_PATH;
    town_tiles[28 * FARM_W + 20] = T_DOOR;

    /* Mine path north. */
    for (int y = 1; y <= 2; y++) town_tiles[y * FARM_W + 20] = T_PATH;
    town_tiles[1 * FARM_W + 20] = T_DOOR;

    /* Shop. */
    for (int y = 5; y <= 6; y++)
        for (int x = 4; x <= 9; x++) town_tiles[y * FARM_W + x] = T_ROOF;
    for (int y = 7; y <= 9; y++)
        for (int x = 4; x <= 9; x++) town_tiles[y * FARM_W + x] = T_WALL;
    town_tiles[7 * FARM_W + 4] = T_WINDOW;
    town_tiles[7 * FARM_W + 9] = T_WINDOW;
    town_tiles[8 * FARM_W + 5] = T_AWNING;
    town_tiles[8 * FARM_W + 6] = T_AWNING;
    town_tiles[8 * FARM_W + 8] = T_AWNING;
    town_tiles[8 * FARM_W + 9] = T_AWNING;
    town_tiles[8 * FARM_W + 7] = T_SIGN;
    town_tiles[9 * FARM_W + 7] = T_DOOR;

    /* Mira home. */
    for (int y = 5; y <= 6; y++)
        for (int x = 13; x <= 18; x++) town_tiles[y * FARM_W + x] = T_ROOF;
    for (int y = 7; y <= 9; y++)
        for (int x = 13; x <= 18; x++) town_tiles[y * FARM_W + x] = T_WALL;
    town_tiles[7 * FARM_W + 13] = T_WINDOW;
    town_tiles[7 * FARM_W + 18] = T_WINDOW;
    town_tiles[9 * FARM_W + 15] = T_DOOR;

    /* Tom home. */
    for (int y = 5; y <= 6; y++)
        for (int x = 23; x <= 28; x++) town_tiles[y * FARM_W + x] = T_ROOF;
    for (int y = 7; y <= 9; y++)
        for (int x = 23; x <= 28; x++) town_tiles[y * FARM_W + x] = T_WALL;
    town_tiles[7 * FARM_W + 23] = T_WINDOW;
    town_tiles[7 * FARM_W + 28] = T_WINDOW;
    town_tiles[9 * FARM_W + 25] = T_DOOR;

    /* Lily home. */
    for (int y = 18; y <= 19; y++)
        for (int x = 6; x <= 11; x++) town_tiles[y * FARM_W + x] = T_ROOF;
    for (int y = 20; y <= 22; y++)
        for (int x = 6; x <= 11; x++) town_tiles[y * FARM_W + x] = T_WALL;
    town_tiles[20 * FARM_W + 6] = T_WINDOW;
    town_tiles[20 * FARM_W + 11] = T_WINDOW;
    town_tiles[22 * FARM_W + 8] = T_DOOR;

    /* Plaza and fountain. */
    for (int y = 12; y <= 16; y++)
        for (int x = 18; x <= 22; x++) town_tiles[y * FARM_W + x] = T_SAND;
    for (int y = 14; y <= 15; y++)
        for (int x = 20; x <= 21; x++) town_tiles[y * FARM_W + x] = T_FOUNTAIN;

    town_tiles[13 * FARM_W + 10] = T_LAMP;
    town_tiles[13 * FARM_W + 30] = T_LAMP;
    town_tiles[11 * FARM_W + 20] = T_LAMP;
    town_tiles[18 * FARM_W + 20] = T_LAMP;

    for (int y = 24; y <= 26; y++)
        for (int x = 3; x <= 5; x++) town_tiles[y * FARM_W + x] = T_TREE;
    for (int y = 22; y <= 25; y++)
        for (int x = 30; x <= 33; x++) town_tiles[y * FARM_W + x] = T_TREE;
    town_tiles[13 * FARM_W + 18] = T_BENCH;
    town_tiles[13 * FARM_W + 22] = T_BENCH;
    town_tiles[17 * FARM_W + 13] = T_FLOWER;
    town_tiles[18 * FARM_W + 25] = T_FLOWER;
}

static void build_room(uint8_t *t, int kind)
{
    uint8_t floor_t = (kind == 0 && house_level >= 2) ? T_FLOOR2 : T_FLOOR;
    uint8_t wall_t = (kind == 0 && house_level >= 2) ? T_WALL_IN2 : T_WALL_IN;
    fill_map(t, INT_W, INT_H, floor_t);
    for (int x = 0; x < INT_W; x++) {
        t[x] = wall_t;
        t[INT_W + x] = wall_t;
        t[2 * INT_W + x] = wall_t;
        t[(INT_H - 1) * INT_W + x] = wall_t;
    }
    for (int y = 0; y < INT_H; y++) {
        t[y * INT_W] = wall_t;
        t[y * INT_W + INT_W - 1] = wall_t;
    }
    t[2 * INT_W + 4] = T_WINDOW_IN;
    t[2 * INT_W + 15] = T_WINDOW_IN;
    t[5 * INT_W + 0] = T_PAINTING;
    t[8 * INT_W + INT_W - 1] = T_CLOCK;
    t[14 * INT_W + 10] = T_DOOR_IN;

    if (kind == 0) {
        t[3 * INT_W + 3] = T_BED;
        t[3 * INT_W + 4] = T_BED;
        t[4 * INT_W + 3] = T_BED2;
        t[4 * INT_W + 4] = T_BED2;
        t[3 * INT_W + 8] = T_TABLE;
        t[3 * INT_W + 7] = T_CHAIR;
        t[3 * INT_W + 9] = T_CHAIR;
        t[3 * INT_W + 15] = T_STOVE;
        t[5 * INT_W + 2] = T_BOOKSHELF;
        t[6 * INT_W + 10] = T_RUG;
        t[6 * INT_W + 11] = T_RUG;
        t[7 * INT_W + 10] = T_RUG;
        t[7 * INT_W + 11] = T_RUG;
        t[4 * INT_W + 17] = T_PLANT;
        t[8 * INT_W + 16] = T_CRATE;
        t[6 * INT_W + 15] = T_BOWL;
        t[6 * INT_W + 16] = T_BOWL2;

        if (house_level >= 1) {
            t[5 * INT_W + 2] = T_BOOKSHELF;
            t[6 * INT_W + 13] = T_RUG;
            t[6 * INT_W + 14] = T_RUG;
            t[7 * INT_W + 13] = T_RUG;
            t[7 * INT_W + 14] = T_RUG;
            t[4 * INT_W + 2] = T_PLANT;
        }
        if (house_level >= 2) {
            t[8 * INT_W + 13] = T_TABLE;
            t[8 * INT_W + 12] = T_CHAIR;
            t[8 * INT_W + 14] = T_CHAIR;
            t[8 * INT_W + 2] = T_STOVE;
            t[9 * INT_W + 17] = T_PLANT;
        }
        if (house_level >= 3) {
            t[5 * INT_W + 7] = T_BOOKSHELF;
            t[9 * INT_W + 2] = T_PLANT;
            t[10 * INT_W + 13] = T_RUG;
            t[10 * INT_W + 14] = T_RUG;
            t[11 * INT_W + 13] = T_RUG;
            t[11 * INT_W + 14] = T_RUG;
        }
        if (house_level >= 4) {
            t[5 * INT_W + 16] = T_PAINTING;
            t[6 * INT_W + 2] = T_CLOCK;
            t[9 * INT_W + 7] = T_PLANT;
            t[10 * INT_W + 3] = T_CRATE;
        }
    } else if (kind == 1) {
        for (int x = 3; x <= 16; x++) t[4 * INT_W + x] = T_COUNTER;
        for (int y = 7; y <= 11; y++) t[y * INT_W + 2] = T_SHELF;
        t[7 * INT_W + 17] = T_SHELF;
        t[8 * INT_W + 17] = T_SHELF;
        t[10 * INT_W + 9] = T_RUG;
        t[10 * INT_W + 10] = T_RUG;
        t[10 * INT_W + 3] = T_CRATE;
        t[10 * INT_W + 16] = T_CRATE;
        t[11 * INT_W + 5] = T_PLANT;
        t[5 * INT_W + 18] = T_SIGN;
    } else {
        t[3 * INT_W + 3] = T_BED;
        t[3 * INT_W + 4] = T_BED;
        t[4 * INT_W + 3] = T_BED2;
        t[4 * INT_W + 4] = T_BED2;
        t[3 * INT_W + 8] = T_TABLE;
        t[3 * INT_W + 7] = T_CHAIR;
        t[3 * INT_W + 9] = T_CHAIR;
        t[5 * INT_W + 16] = T_BOOKSHELF;
        t[6 * INT_W + 11] = T_RUG;
        t[6 * INT_W + 12] = T_RUG;
        t[7 * INT_W + 11] = T_RUG;
        t[7 * INT_W + 12] = T_RUG;
        t[4 * INT_W + 17] = T_PLANT;
    }
}

static void build_beach(void)
{
    for (int y = 0; y < FARM_H; y++)
        for (int x = 0; x < FARM_W; x++)
            beach_tiles[y * FARM_W + x] = T_SAND;

    for (int y = 7; y < FARM_H; y++)
        for (int x = 24; x < FARM_W; x++)
            beach_tiles[y * FARM_W + x] = T_WATER;

    for (int y = 2; y <= 13; y++)
        beach_tiles[y * FARM_W + 20] = T_PATH;
    for (int y = 14; y <= 15; y++)
        for (int x = 20; x <= 31; x++)
            beach_tiles[y * FARM_W + x] = T_PIER;

    /* Hut and rod sign. */
    for (int y = 4; y <= 5; y++)
        for (int x = 4; x <= 8; x++) beach_tiles[y * FARM_W + x] = T_ROOF;
    for (int y = 6; y <= 7; y++)
        for (int x = 4; x <= 8; x++) beach_tiles[y * FARM_W + x] = T_WALL;
    beach_tiles[7 * FARM_W + 6] = T_SIGN;

    beach_tiles[10 * FARM_W + 16] = T_ROCK;
    beach_tiles[24 * FARM_W + 10] = T_ROCK;
    beach_tiles[2 * FARM_W + 20] = T_DOOR;
}

static void build_mine(void)
{
    fill_map(mine_tiles, INT_W, INT_H, T_MINE_FLOOR);
    for (int x = 0; x < INT_W; x++) {
        mine_tiles[x] = T_MINE_WALL;
        mine_tiles[(INT_H - 1) * INT_W + x] = T_MINE_WALL;
    }
    for (int y = 0; y < INT_H; y++) {
        mine_tiles[y * INT_W] = T_MINE_WALL;
        mine_tiles[y * INT_W + INT_W - 1] = T_MINE_WALL;
    }
    const uint8_t rocks[][2] = {{4,4},{6,7},{12,5},{14,9},{5,10},{15,4}};
    for (unsigned i = 0; i < sizeof(rocks) / 2; i++) {
        mine_tiles[rocks[i][1] * INT_W + rocks[i][0]] = T_ROCK;
    }
    mine_tiles[4 * INT_W + 8] = T_ORE;
    mine_tiles[7 * INT_W + 15] = T_ORE;
    mine_tiles[10 * INT_W + 5] = T_ORE;
    mine_tiles[13 * INT_W + 10] = T_DOOR;
    mine_tiles[13 * INT_W + 9] = T_LADDER;
}

static void build_world(void)
{
    build_farm();
    build_town();
    build_room(home_tiles, 0);
    build_room(shop_tiles, 1);
    build_room(mira_tiles, 2);
    build_room(tom_tiles, 2);
    build_room(lily_tiles, 2);
    build_beach();
    build_mine();
}

static int mine_unlocked(void)
{
    return story[0] >= 4 && story[1] >= 4 && story[2] >= 4;
}

static int all_stories_done(void)
{
    return story[0] >= 4 && story[1] >= 4 && story[2] >= 4;
}

static void mine_reset(void)
{
    static const uint8_t sx[3] = {6, 13, 8};
    static const uint8_t sy[3] = {6, 8, 11};
    for (int i = 0; i < 3; i++) {
        slime_x[i] = sx[i] * TILE;
        slime_y[i] = sy[i] * TILE;
        slime_hp[i] = 2;
        slime_alive[i] = 1;
    }
    slime_hit_cd = 0;
}

static void scene_set(int s)
{
    if (s < 0 || s >= SC_COUNT) s = SC_FARM;
    scene_id = s;
    if (s == SC_MINE) mine_reset();
}

static const Door *door_at(int scene, int x, int y)
{
    for (int i = 0; i < DOOR_COUNT; i++) {
        if (doors[i].scene == scene && doors[i].x == x && doors[i].y == y) return &doors[i];
    }
    return 0;
}

static const Door *door_adjacent(void)
{
    int px = (player_x + 8) / TILE;
    int py = (player_y + 12) / TILE;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            const Door *d = door_at(scene_id, px + dx, py + dy);
            if (d) return d;
        }
    }
    return 0;
}

static int plot_index(int x, int y)
{
    if (x < 11 || x > 16 || y < 8 || y > 11) return -1;
    return (y - 8) * 6 + (x - 11);
}

static uint8_t map_get(int x, int y)
{
    if ((unsigned)x >= (unsigned)scene_w[scene_id] ||
        (unsigned)y >= (unsigned)scene_h[scene_id]) return T_WALL;
    return scene_tiles[scene_id][y * scene_w[scene_id] + x];
}

static int cat_on_bed(void)
{
    return (pet_owned & 1) && pet_aff[0] >= 60 && scene_id == SC_HOME && minutes >= 21 * 60;
}

static int player_near_bed(void)
{
    if (scene_id != SC_HOME) return 0;
    int px = (player_x + 8) / TILE;
    int py = (player_y + 12) / TILE;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            uint8_t bt = map_get(px + dx, py + dy);
            if (bt == T_BED || bt == T_BED2) return 1;
        }
    }
    return 0;
}

static int shell_near(void)
{
    if (scene_id != SC_BEACH) return -1;
    for (int i = 0; i < 4; i++) {
        if (shell_mask & (1 << i)) continue;
        int dx = iabs(shell_x[i] * TILE + 8 - (player_x + 8));
        int dy = iabs(shell_y[i] * TILE + 8 - (player_y + 8));
        if (dx < 18 && dy < 18) return i;
    }
    return -1;
}

/* --- actors ------------------------------------------------------------ */

typedef struct {
    uint16_t skin, hair, shirt, pants, shoes, accent, hat;
    uint8_t dress;
} Look;

static const Look looks[4] = {
    /* player: straw hat, blue shirt, overalls */
    { RGB(240, 200, 160), RGB(74, 42, 26), RGB(58, 90, 138), RGB(74, 58, 42),
      RGB(30, 26, 22), RGB(255, 255, 255), 0, 0 },
    /* mira: long auburn hair, red dress, white apron */
    { RGB(246, 206, 170), RGB(120, 58, 34), RGB(190, 62, 74), RGB(140, 60, 70),
      RGB(80, 40, 30), RGB(255, 255, 255), 0, 1 },
    /* tom: gray hair, green shirt, cap */
    { RGB(232, 190, 150), RGB(180, 180, 180), RGB(70, 120, 70), RGB(80, 64, 48),
      RGB(30, 26, 22), RGB(200, 160, 80), RGB(90, 90, 90), 0 },
    /* lily: blonde hair, pink dress, bow */
    { RGB(250, 210, 180), RGB(232, 196, 96), RGB(230, 130, 160), RGB(200, 100, 130),
      RGB(120, 60, 80), RGB(255, 255, 255), RGB(240, 80, 120), 1 },
};

static void p16(uint16_t *fb, int sx, int sy, int dir, int x, int y, uint16_t c)
{
    if (dir == 2) x = 15 - x;
    put_px(fb, sx + x, sy + y, c);
}

static void r16(uint16_t *fb, int sx, int sy, int dir, int x, int y, int w, int h, uint16_t c)
{
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            p16(fb, sx, sy, dir, x + i, y + j, c);
        }
    }
}

static void draw_person(uint16_t *fb, int sx, int sy, int style, int dir, int frame)
{
    const Look *L = &looks[style];
    int step = frame & 3;
    int lo = (step == 1) ? 1 : (step == 3 ? -1 : 0);
    int ro = -lo;
    int ax = (step == 1 || step == 3) ? 1 : 0;
    int oy = sy - 8; /* sprite is 16x24, feet aligned with the 16x16 tile */

    /* shadow */
    r16(fb, sx, sy, dir, 3, 15, 10, 2, RGB(36, 32, 28));

    /* silhouette */
    uint16_t out = RGB(24, 20, 18);
    r16(fb, sx, oy, dir, 2, 1, 12, 10, out);
    r16(fb, sx, oy, dir, 2, 9, 12, 9, out);
    r16(fb, sx, oy, dir, 1, 9, 3, 8, out);
    r16(fb, sx, oy, dir, 12, 9, 3, 8, out);
    r16(fb, sx, oy, dir, 2, 16, 12, 8, out);

    /* legs */
    if (L->dress) {
        r16(fb, sx, oy, dir, 3 + lo, 17, 4, 4, L->skin);
        r16(fb, sx, oy, dir, 9 + ro, 17, 4, 4, L->skin);
    } else {
        r16(fb, sx, oy, dir, 3 + lo, 17, 4, 5, L->pants);
        r16(fb, sx, oy, dir, 9 + ro, 17, 4, 5, L->pants);
    }
    r16(fb, sx, oy, dir, 3 + lo, 22, 4, 1, L->shoes);
    r16(fb, sx, oy, dir, 9 + ro, 22, 4, 1, L->shoes);

    /* body */
    r16(fb, sx, oy, dir, 3, 10, 10, 7, L->shirt);
    if (L->dress) {
        r16(fb, sx, oy, dir, 4, 11, 8, 5, L->accent);
        r16(fb, sx, oy, dir, 4, 15, 8, 1, L->pants);
    } else {
        r16(fb, sx, oy, dir, 4, 13, 8, 4, L->pants);
        r16(fb, sx, oy, dir, 5, 10, 1, 3, L->pants);
        r16(fb, sx, oy, dir, 10, 10, 1, 3, L->pants);
        r16(fb, sx, oy, dir, 7, 13, 2, 1, L->accent);
    }

    /* arms */
    int ay = 10 + ax;
    r16(fb, sx, oy, dir, 1, ay, 3, 6, L->shirt);
    r16(fb, sx, oy, dir, 12, ay, 3, 6, L->shirt);
    r16(fb, sx, oy, dir, 1, ay + 5, 3, 1, L->skin);
    r16(fb, sx, oy, dir, 12, ay + 5, 3, 1, L->skin);

    /* head */
    r16(fb, sx, oy, dir, 3, 2, 10, 8, L->skin);

    /* ears */
    if (dir == 0) {
        r16(fb, sx, oy, dir, 2, 5, 1, 2, L->skin);
        r16(fb, sx, oy, dir, 13, 5, 1, 2, L->skin);
    }

    /* hair */
    if (dir == 1) {
        r16(fb, sx, oy, dir, 3, 2, 10, 7, L->hair);
        r16(fb, sx, oy, dir, 3, 9, 10, 1, L->hair);
    } else if (dir == 2 || dir == 3) {
        r16(fb, sx, oy, dir, 3, 2, 10, 3, L->hair);
        r16(fb, sx, oy, dir, 2, 4, 2, 6, L->hair);
        r16(fb, sx, oy, dir, 3, 5, 1, 3, L->hair);
    } else {
        r16(fb, sx, oy, dir, 3, 2, 10, 3, L->hair);
        r16(fb, sx, oy, dir, 3, 5, 1, 3, L->hair);
        r16(fb, sx, oy, dir, 12, 5, 1, 3, L->hair);
    }

    if (style == 1 || style == 3) {
        r16(fb, sx, oy, dir, 2, 5, 1, 8, L->hair);
        r16(fb, sx, oy, dir, 13, 5, 1, 8, L->hair);
    }

    if (style == 0) {
        uint16_t hi = RGB(112, 66, 38);
        r16(fb, sx, oy, dir, 5, 1, 2, 1, L->hair);
        r16(fb, sx, oy, dir, 9, 1, 2, 1, L->hair);
        r16(fb, sx, oy, dir, 4, 2, 2, 1, L->hair);
        r16(fb, sx, oy, dir, 10, 2, 2, 1, L->hair);
        if (dir == 1) {
            r16(fb, sx, oy, dir, 4, 3, 8, 2, L->hair);
            r16(fb, sx, oy, dir, 3, 5, 10, 2, L->hair);
            r16(fb, sx, oy, dir, 4, 7, 8, 1, L->hair);
            r16(fb, sx, oy, dir, 5, 8, 6, 1, L->hair);
            r16(fb, sx, oy, dir, 2, 5, 2, 3, L->hair);
            r16(fb, sx, oy, dir, 12, 5, 2, 3, L->hair);
            r16(fb, sx, oy, dir, 5, 3, 2, 1, hi);
            r16(fb, sx, oy, dir, 7, 5, 3, 1, hi);
            r16(fb, sx, oy, dir, 4, 6, 1, 2, hi);
            r16(fb, sx, oy, dir, 10, 4, 1, 2, hi);
            r16(fb, sx, oy, dir, 6, 9, 4, 1, L->skin);
            put_px(fb, sx + 2, oy + 6, L->skin);
            put_px(fb, sx + 13, oy + 6, L->skin);
            put_px(fb, sx + 3, oy + 8, L->hair);
            put_px(fb, sx + 5, oy + 8, L->hair);
            put_px(fb, sx + 8, oy + 8, L->hair);
            put_px(fb, sx + 10, oy + 8, L->hair);
            put_px(fb, sx + 12, oy + 8, L->hair);
        } else if (dir == 0) {
            r16(fb, sx, oy, dir, 3, 3, 10, 2, L->hair);
            r16(fb, sx, oy, dir, 2, 4, 2, 4, L->hair);
            r16(fb, sx, oy, dir, 12, 4, 2, 4, L->hair);
            r16(fb, sx, oy, dir, 5, 3, 2, 1, hi);
            r16(fb, sx, oy, dir, 6, 9, 4, 1, L->skin);
        } else {
            r16(fb, sx, oy, dir, 2, 4, 2, 4, L->hair);
            r16(fb, sx, oy, dir, 4, 3, 3, 1, hi);
            r16(fb, sx, oy, dir, 6, 9, 4, 1, L->skin);
        }
    }

    /* hat / bow */
    if (L->hat) {
        if (style == 0) {
            r16(fb, sx, oy, dir, 2, 2, 12, 2, L->hat);
            r16(fb, sx, oy, dir, 4, 0, 8, 2, L->hat);
            r16(fb, sx, oy, dir, 4, 2, 8, 1, L->accent);
        } else if (style == 2) {
            r16(fb, sx, oy, dir, 1, 1, 14, 1, L->hat);
            r16(fb, sx, oy, dir, 3, 0, 10, 1, L->hat);
        } else if (style == 3) {
            r16(fb, sx, oy, dir, 11, 1, 4, 3, L->hat);
            r16(fb, sx, oy, dir, 10, 2, 1, 1, L->hat);
        }
    }

    /* face */
    if (dir == 0) {
        r16(fb, sx, oy, dir, 5, 5, 2, 2, RGB(30, 26, 22));
        r16(fb, sx, oy, dir, 9, 5, 2, 2, RGB(30, 26, 22));
        r16(fb, sx, oy, dir, 7, 7, 2, 1, RGB(150, 80, 70));
        put_px(fb, sx + 4, oy + 7, RGB(220, 150, 130));
        put_px(fb, sx + 11, oy + 7, RGB(220, 150, 130));
    } else if (dir == 2) {
        r16(fb, sx, oy, dir, 5, 5, 2, 2, RGB(30, 26, 22));
        r16(fb, sx, oy, dir, 4, 6, 1, 1, RGB(210, 160, 130));
    } else if (dir == 3) {
        r16(fb, sx, oy, dir, 9, 5, 2, 2, RGB(30, 26, 22));
        r16(fb, sx, oy, dir, 11, 6, 1, 1, RGB(210, 160, 130));
    }
    if (style == 0) {
        r16(fb, sx, oy, dir, 4, 10, 8, 1, RGB(224, 224, 230));
        r16(fb, sx, oy, dir, 5, 11, 6, 1, RGB(52, 78, 120));
    }
}

static void draw_player(uint16_t *fb, int sx, int sy)
{
    draw_person(fb, sx, sy, 0, facing, player_walk);
}

static void draw_npc(uint16_t *fb, int sx, int sy, int id)
{
    int wf = npcs[id].moving ? ((npcs[id].frame / 4) & 3) : 0;
    draw_person(fb, sx, sy, id + 1, npcs[id].dir, wf);
    if (id == 2 && story[2] >= 4) {
        int yy = sy - 11;
        fill_rect(fb, sx + 3, yy + 1, 2, 2, C_ROSE);
        fill_rect(fb, sx + 7, yy, 2, 2, C_GOLD);
        fill_rect(fb, sx + 11, yy + 1, 2, 2, C_ROSE);
        fill_rect(fb, sx + 4, yy, 1, 1, C_LEAF);
        fill_rect(fb, sx + 10, yy, 1, 1, C_LEAF);
    }
}

/* --- crops ------------------------------------------------------------- */

static int crop_stage(int id, int growth)
{
    if (growth <= 0) return 0;
    int days = crops[id].days;
    if (days <= 0) return 0;
    if (growth >= days) return 3;
    int s = 1 + (growth * 2) / days;
    if (s > 2) s = 2;
    return s;
}

static void draw_crop(uint16_t *fb, int sx, int sy, int id, int growth, int water)
{
    int stage = crop_stage(id, growth);
    uint16_t fruit = crops[id].fruit;

    if (water) {
        put_px(fb, sx + 13, sy + 3, C_WATER_L);
        put_px(fb, sx + 12, sy + 4, C_WATER_L);
    }

    fill_rect(fb, sx + 5, sy + 13, 6, 2, C_SOIL_D);

    if (stage == 0) {
        fill_rect(fb, sx + 6, sy + 9, 4, 4, C_LEAF);
        fill_rect(fb, sx + 7, sy + 8, 2, 2, C_LEAF_L);
    } else if (stage == 1) {
        fill_rect(fb, sx + 4, sy + 8, 8, 5, C_LEAF);
        fill_rect(fb, sx + 5, sy + 7, 6, 3, C_LEAF_L);
        fill_rect(fb, sx + 7, sy + 11, 2, 3, C_LEAF_D);
    } else if (stage == 2) {
        fill_rect(fb, sx + 3, sy + 6, 10, 7, C_LEAF);
        fill_rect(fb, sx + 4, sy + 5, 8, 3, C_LEAF_L);
        fill_rect(fb, sx + 7, sy + 11, 2, 3, C_LEAF_D);
    } else {
        fill_rect(fb, sx + 2, sy + 4, 12, 9, C_LEAF);
        fill_rect(fb, sx + 3, sy + 3, 10, 4, C_LEAF_L);
        fill_rect(fb, sx + 5, sy + 6, 6, 6, C_LEAF_L);
        int kind = id & 3;
        if (kind == 0) {
            fill_rect(fb, sx + 6, sy + 6, 4, 6, fruit);
            fill_rect(fb, sx + 5, sy + 10, 2, 2, fruit);
            fill_rect(fb, sx + 9, sy + 10, 2, 2, fruit);
            put_px(fb, sx + 7, sy + 7, C_WHITE);
            fill_rect(fb, sx + 7, sy + 2, 2, 3, C_LEAF_D);
        } else if (kind == 1) {
            fill_rect(fb, sx + 5, sy + 7, 6, 5, fruit);
            fill_rect(fb, sx + 4, sy + 8, 2, 3, fruit);
            fill_rect(fb, sx + 10, sy + 8, 2, 3, fruit);
            put_px(fb, sx + 6, sy + 8, RGB(230, 190, 150));
        } else if (kind == 2) {
            fill_rect(fb, sx + 5, sy + 6, 6, 6, fruit);
            fill_rect(fb, sx + 6, sy + 5, 4, 2, RGB(130, 190, 100));
            put_px(fb, sx + 7, sy + 7, C_LEAF_L);
            put_px(fb, sx + 9, sy + 9, C_LEAF_D);
        } else {
            fill_rect(fb, sx + 5, sy + 6, 6, 6, RGB(70, 130, 70));
            put_px(fb, sx + 6, sy + 7, fruit);
            put_px(fb, sx + 9, sy + 8, fruit);
            put_px(fb, sx + 7, sy + 10, fruit);
            put_px(fb, sx + 10, sy + 11, fruit);
        }
        fill_rect(fb, sx + 7, sy + 12, 2, 3, C_LEAF_D);
        put_px(fb, sx + 1, sy + 3, C_GOLD);
        put_px(fb, sx + 14, sy + 5, C_GOLD);
    }
}

/* --- npc --------------------------------------------------------------- */

static const Sched *npc_sched(int i)
{
    if (i == 0) return sched_mira;
    if (i == 1) return sched_tom;
    return sched_lily;
}

static int npc_update(int i)
{
    const Sched *s = npc_sched(i);
    while (s[1].start != 1440 && s[1].start <= (uint16_t)minutes) s++;
    int sx = s->x * TILE;
    int sy = s->y * TILE;

    if (npcs[i].scene != (int16_t)s->scene) {
        npcs[i].scene = s->scene;
        npcs[i].x = sx;
        npcs[i].y = sy;
        npcs[i].moving = 0;
        npcs[i].wander = 0;
        npcs[i].wait = 0;
        return 1;
    }

    int dx = sx - npcs[i].x;
    int dy = sy - npcs[i].y;
    if (dx != 0 || dy != 0) {
        npcs[i].step++;
        if ((npcs[i].step & 3) != 0) return 0;
        npcs[i].moving = 1;
        npcs[i].wander = 0;
        if (dx) {
            npcs[i].x += (dx > 0) ? 1 : -1;
            npcs[i].dir = (dx > 0) ? 3 : 2;
        } else {
            npcs[i].y += (dy > 0) ? 1 : -1;
            npcs[i].dir = (dy > 0) ? 0 : 1;
        }
        npcs[i].frame++;
        return 1;
    }

    if (npcs[i].wait > 0) {
        npcs[i].wait--;
        npcs[i].moving = 0;
        return 0;
    }

    if (!npcs[i].wander) {
        int ox = (rnd() % 3 - 1) * TILE;
        int oy = (rnd() % 3 - 1) * TILE;
        int wx = sx + ox;
        int wy = sy + oy;
        int tx = wx / TILE;
        int ty = wy / TILE;
        if (wx < 0 || wy < 0 || tx >= scene_w[npcs[i].scene] || ty >= scene_h[npcs[i].scene] ||
            solid(scene_tiles[npcs[i].scene][ty * scene_w[npcs[i].scene] + tx])) {
            wx = sx;
            wy = sy;
        }
        npcs[i].wx = (int16_t)wx;
        npcs[i].wy = (int16_t)wy;
        npcs[i].wander = 1;
    }

    dx = npcs[i].wx - npcs[i].x;
    dy = npcs[i].wy - npcs[i].y;
    if (dx == 0 && dy == 0) {
        npcs[i].wander = 0;
        npcs[i].wait = (uint16_t)(60 + rnd() % 120);
        npcs[i].moving = 0;
        return 0;
    }

    npcs[i].step++;
    if ((npcs[i].step & 3) != 0) return 0;
    npcs[i].moving = 1;
    if (dx) {
        npcs[i].x += (dx > 0) ? 1 : -1;
        npcs[i].dir = (dx > 0) ? 3 : 2;
    } else {
        npcs[i].y += (dy > 0) ? 1 : -1;
        npcs[i].dir = (dy > 0) ? 0 : 1;
    }
    npcs[i].frame++;
    return 1;
}

static int iabs(int v) { return v < 0 ? -v : v; }

static int npc_near(void)
{
    int best = -1;
    int bestd = 80;
    for (int i = 0; i < 3; i++) {
        if (npcs[i].scene != scene_id) continue;
        int dx = iabs((npcs[i].x + 8) - (player_x + 8));
        int dy = iabs((npcs[i].y + 8) - (player_y + 8));
        if (dx < 56 && dy < 56 && dx + dy < 80 && dx + dy < bestd) {
            best = i;
            bestd = dx + dy;
        }
    }
    return best;
}

static void draw_hearts(uint16_t *fb, int x, int y, int points)
{
    int full = points / 20;
    int half = (points % 20) >= 10;
    for (int i = 0; i < 5; i++) {
        uint16_t c = (i < full) ? RGB(230, 70, 90) : RGB(90, 90, 90);
        fill_rect(fb, x + i * 14, y + 2, 4, 4, c);
        fill_rect(fb, x + i * 14 + 4, y + 2, 4, 4, c);
        fill_rect(fb, x + i * 14 + 2, y, 4, 2, c);
        put_px(fb, x + i * 14 + 3, y + 6, c);
        put_px(fb, x + i * 14 + 4, y + 6, c);
        if (i == full && half) {
            fill_rect(fb, x + i * 14, y + 2, 4, 4, RGB(230, 70, 90));
            fill_rect(fb, x + i * 14 + 2, y, 2, 2, RGB(230, 70, 90));
        }
    }
}

static void draw_shell(uint16_t *fb, int sx, int sy)
{
    fill_rect(fb, sx + 4, sy + 8, 8, 5, RGB(238, 150, 170));
    fill_rect(fb, sx + 4, sy + 6, 8, 3, RGB(250, 210, 210));
    fill_rect(fb, sx + 5, sy + 5, 6, 2, RGB(255, 235, 235));
    put_px(fb, sx + 5, sy + 9, RGB(190, 110, 140));
    put_px(fb, sx + 10, sy + 9, RGB(190, 110, 140));
}

/* --- ui ---------------------------------------------------------------- */

static int sel_count(void)
{
    if (sel_kind == IT_SEED) return seeds[sel_id];
    if (sel_kind == IT_CROP) return produce[sel_id];
    if (sel_kind == IT_FISH) return fish[sel_id];
    return 1;
}

static const char *item_name(int kind, int id)
{
    if (kind == IT_HOE) return "hoe";
    if (kind == IT_CAN) return "can";
    if (kind == IT_ROD) return "rod";
    if (kind == IT_SWORD) return "sword";
    if (kind == IT_SEED || kind == IT_CROP) return crops[id].name;
    if (kind == IT_FISH) return fish_name[id];
    return "bag";
}

static const char *item_name_full(int kind, int id)
{
    static char buf[24];
    if (kind != IT_SEED) return item_name(kind, id);
    int p = 0;
    const char *n = crops[id].name;
    while (*n && p < 18) buf[p++] = *n++;
    buf[p++] = ' ';
    buf[p++] = 's';
    buf[p++] = 'e';
    buf[p++] = 'e';
    buf[p++] = 'd';
    buf[p] = 0;
    return buf;
}

static const char *sel_name(void)
{
    return item_name(sel_kind, sel_id);
}

static void draw_item_icon(uint16_t *fb, int x, int y, int id, int seed)
{
    uint16_t c = crops[id].fruit;
    uint16_t dark = RGB(58, 42, 30);
    if (seed) {
        fill_rect(fb, x + 1, y + 2, 14, 13, dark);
        fill_rect(fb, x + 2, y + 3, 12, 11, RGB(214, 184, 132));
        fill_rect(fb, x + 3, y + 4, 10, 3, c);
        fill_rect(fb, x + 4, y + 8, 8, 4, RGB(236, 216, 172));
        fill_rect(fb, x + 5, y + 9, 2, 2, c);
        fill_rect(fb, x + 8, y + 9, 2, 2, RGB(110, 82, 48));
        fill_rect(fb, x + 5, y + 1, 6, 2, RGB(150, 104, 62));
        fill_rect(fb, x + 4, y + 0, 2, 2, RGB(120, 80, 44));
        fill_rect(fb, x + 10, y + 0, 2, 2, RGB(120, 80, 44));
        put_px(fb, x + 3, y + 4, RGB(255, 242, 208));
        return;
    }

    if (id == 14) {
        fill_rect(fb, x + 3, y + 3, 10, 10, RGB(86, 76, 62));
        fill_rect(fb, x + 4, y + 4, 8, 8, RGB(226, 214, 184));
        fill_rect(fb, x + 5, y + 5, 6, 2, RGB(246, 238, 214));
        fill_rect(fb, x + 5, y + 8, 6, 2, RGB(246, 238, 214));
        put_px(fb, x + 8, y + 7, RGB(120, 104, 82));
        put_px(fb, x + 9, y + 6, RGB(120, 104, 82));
        return;
    }
    if (id == 15) {
        fill_rect(fb, x + 4, y + 3, 8, 10, RGB(246, 240, 218));
        fill_rect(fb, x + 5, y + 2, 6, 2, RGB(255, 252, 236));
        fill_rect(fb, x + 5, y + 4, 2, 3, C_WHITE);
        fill_rect(fb, x + 7, y + 9, 4, 2, RGB(228, 218, 190));
        return;
    }

    int kind = id & 3;
    fill_rect(fb, x + 2, y + 2, 12, 13, dark);
    if (kind == 0) {
        fill_rect(fb, x + 6, y + 6, 4, 7, c);
        fill_rect(fb, x + 5, y + 9, 6, 3, c);
        fill_rect(fb, x + 7, y + 12, 2, 2, c);
        fill_rect(fb, x + 4, y + 4, 2, 2, RGB(88, 150, 72));
        fill_rect(fb, x + 7, y + 3, 2, 3, RGB(76, 138, 64));
        fill_rect(fb, x + 10, y + 4, 2, 2, RGB(104, 168, 80));
        fill_rect(fb, x + 7, y + 6, 2, 2, RGB(250, 170, 96));
        put_px(fb, x + 5, y + 10, RGB(255, 214, 150));
    } else if (kind == 1) {
        fill_rect(fb, x + 4, y + 5, 8, 7, c);
        fill_rect(fb, x + 3, y + 7, 2, 4, c);
        fill_rect(fb, x + 11, y + 7, 2, 4, c);
        fill_rect(fb, x + 5, y + 6, 3, 2, RGB(200, 150, 104));
        put_px(fb, x + 6, y + 8, RGB(80, 54, 36));
        put_px(fb, x + 10, y + 9, RGB(80, 54, 36));
        fill_rect(fb, x + 8, y + 4, 2, 1, RGB(84, 144, 66));
    } else if (kind == 2) {
        fill_rect(fb, x + 4, y + 4, 8, 8, c);
        fill_rect(fb, x + 3, y + 6, 2, 4, c);
        fill_rect(fb, x + 11, y + 6, 2, 4, c);
        fill_rect(fb, x + 5, y + 3, 5, 2, RGB(118, 174, 94));
        fill_rect(fb, x + 5, y + 6, 3, 3, RGB(150, 196, 112));
        fill_rect(fb, x + 8, y + 8, 2, 3, RGB(84, 142, 72));
        put_px(fb, x + 9, y + 6, RGB(216, 238, 182));
    } else {
        fill_rect(fb, x + 4, y + 5, 8, 7, RGB(72, 128, 68));
        fill_rect(fb, x + 5, y + 6, 2, 2, c);
        fill_rect(fb, x + 8, y + 5, 2, 2, c);
        fill_rect(fb, x + 7, y + 8, 2, 2, c);
        fill_rect(fb, x + 10, y + 9, 2, 2, c);
        fill_rect(fb, x + 5, y + 10, 2, 2, c);
        fill_rect(fb, x + 8, y + 3, 2, 2, RGB(96, 156, 78));
        put_px(fb, x + 6, y + 7, RGB(255, 220, 220));
    }
}
static void draw_energy_icon(uint16_t *fb, int x, int y)
{
    fill_rect(fb, x + 2, y, 4, 4, C_ENERGY);
    fill_rect(fb, x + 1, y + 3, 4, 3, C_ENERGY);
    fill_rect(fb, x + 3, y + 2, 2, 4, C_ENERGY_D);
}

static void draw_water_icon(uint16_t *fb, int x, int y)
{
    fill_rect(fb, x + 3, y, 2, 2, C_WATER_L);
    fill_rect(fb, x + 2, y + 2, 4, 3, C_WATER_L);
    fill_rect(fb, x + 1, y + 5, 6, 2, C_WATER);
    put_px(fb, x + 3, y + 3, C_WHITE);
}

static void draw_hud(uint16_t *fb)
{
    fill_rect(fb, 0, 0, GAME_W, 40, C_HUD);
    fill_rect(fb, 0, 39, GAME_W, 1, C_HUD_L);

    draw_text_big_shadow(fb, 4, 3, season_name[season], C_WHITE);
    draw_num_big_shadow(fb, 82, 3, day, C_WHITE);
    draw_time_big(fb, 110, 3, minutes);

    int n = 1, v = coins;
    while (v >= 10) { n++; v /= 10; }
    draw_num_big_shadow(fb, GAME_W - 16 - n * 12, 3, coins, C_GOLD);
    draw_text_big_shadow(fb, GAME_W - 14, 3, "g", C_GOLD);

    draw_energy_icon(fb, 3, 21);
    fill_rect(fb, 13, 22, 74, 6, C_ENERGY_D);
    int w = energy * 70 / energy_max;
    if (w < 0) w = 0;
    if (w > 70) w = 70;
    fill_rect(fb, 15, 23, w, 4, C_ENERGY);

    draw_water_icon(fb, 3, 30);
    fill_rect(fb, 13, 31, 74, 4, C_WATER_D);
    int ww = water_can * 70 / water_can_max;
    if (ww < 0) ww = 0;
    if (ww > 70) ww = 70;
    fill_rect(fb, 15, 32, ww, 2, C_WATER_L);

    if (sel_kind == IT_HOE || sel_kind == IT_CAN || sel_kind == IT_ROD || sel_kind == IT_SWORD) {
        draw_tool_icon(fb, 87, 23, sel_kind);
    } else if (sel_kind == IT_FISH) {
        draw_fish_icon(fb, 87, 23, sel_id);
    } else {
        draw_item_icon(fb, 87, 23, sel_id, sel_kind == IT_SEED);
    }
    draw_text_big_shadow(fb, 108, 21, sel_name(), C_WHITE);
    if (sel_kind == IT_SEED || sel_kind == IT_CROP || sel_kind == IT_FISH) {
        draw_num_big_shadow(fb, 200, 21, sel_count(), C_WHITE);
    }
    if (scene_id == SC_MINE) {
        draw_text_big_shadow(fb, 230, 21, "ore", C_WHITE);
        draw_num_big_shadow(fb, 270, 21, ore_count, C_GOLD);
        draw_hearts(fb, 4, 44, player_hp * 20);
    } else {
        draw_text_big_shadow(fb, 230, 21, "bin", C_WHITE);
        draw_num_big_shadow(fb, 270, 21, bin_value, C_GOLD);
    }
}

static void draw_message(uint16_t *fb)
{
    if (!message || message_timer <= 0) return;
    int len = 0;
    while (message[len]) len++;
    int w = len * 12 + 16;
    int x = (GAME_W - w) / 2;
    int y = 190;
    fill_rect(fb, x, y, w, 26, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 8, y + 5, message, C_WHITE);
}

static void draw_shop(uint16_t *fb)
{
    int x = 16, y = 44, w = 288, h = 184;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 16, y + 8, "seed shop", C_GOLD);

    int n = 1, v = coins;
    while (v >= 10) { n++; v /= 10; }
    draw_num_big_shadow(fb, x + w - 24 - n * 12, y + 8, coins, C_GOLD);
    draw_text_big_shadow(fb, x + w - 22, y + 8, "g", C_GOLD);

    int count = season_crop_count[season];
    for (int i = 0; i < count; i++) {
        int id = season * 4 + i;
        int ry = y + 30 + i * 24;
        if (i == shop_sel) fill_rect(fb, x + 8, ry - 4, w - 16, 22, C_HUD_L);
        draw_text_big_shadow(fb, x + 16, ry, crops[id].name, C_WHITE);
        draw_num_big_shadow(fb, x + 136, ry, crops[id].price, C_GOLD);
        draw_text_big_shadow(fb, x + 160, ry, "g", C_GOLD);
        draw_text_big_shadow(fb, x + 184, ry, "have", C_WHITE);
        draw_num_big_shadow(fb, x + 248, ry, seeds[id], C_WHITE);
    }

    int id = season * 4 + shop_sel;
    draw_text_big_shadow(fb, x + 16, y + 132, "days", C_WHITE);
    draw_num_big_shadow(fb, x + 72, y + 132, crops[id].days, C_WHITE);
    draw_text_big_shadow(fb, x + 112, y + 132, "sell", C_WHITE);
    draw_num_big_shadow(fb, x + 168, y + 132, crops[id].sell, C_GOLD);
    draw_text_big_shadow(fb, x + 192, y + 132, "g", C_GOLD);
    if (crops[id].regrow) {
        draw_text_big_shadow(fb, x + 216, y + 132, "regrow", C_GOLD);
    }

    draw_text_big_shadow(fb, x + 16, y + 156, "qty", C_WHITE);
    draw_num_big_shadow(fb, x + 56, y + 156, shop_qty, C_GOLD);
    draw_text_big_shadow(fb, x + 96, y + 156, "A buy", C_WHITE);
    draw_text_big_shadow(fb, x + 196, y + 156, "B exit", C_WHITE);
}

static void draw_face(uint16_t *fb, int x, int y, int id)
{
    const Look *L = &looks[id + 1];
    fill_rect(fb, x, y, 32, 32, C_GOLD);
    fill_rect(fb, x + 1, y + 1, 30, 30, C_HUD);
    fill_rect(fb, x + 2, y + 2, 28, 28, RGB(72, 54, 40));

    fill_rect(fb, x + 5, y + 5, 22, 25, L->hair);
    fill_rect(fb, x + 7, y + 7, 18, 17, L->skin);
    fill_rect(fb, x + 6, y + 5, 20, 6, L->hair);
    fill_rect(fb, x + 5, y + 10, 2, 10, L->hair);
    fill_rect(fb, x + 25, y + 10, 2, 10, L->hair);
    fill_rect(fb, x + 6, y + 15, 1, 4, L->skin);
    fill_rect(fb, x + 25, y + 15, 1, 4, L->skin);

    if (id == 0 || id == 2) {
        fill_rect(fb, x + 5, y + 25, 4, 3, L->hair);
        fill_rect(fb, x + 23, y + 25, 4, 3, L->hair);
    }

    if (id == 1) {
        fill_rect(fb, x + 4, y + 3, 24, 3, L->hat);
        fill_rect(fb, x + 6, y + 1, 20, 2, L->hat);
    } else if (id == 2) {
        fill_rect(fb, x + 23, y + 3, 5, 4, L->hat);
        fill_rect(fb, x + 22, y + 4, 1, 1, L->hat);
    }

    fill_rect(fb, x + 11, y + 14, 3, 4, RGB(30, 26, 22));
    fill_rect(fb, x + 18, y + 14, 3, 4, RGB(30, 26, 22));
    fill_rect(fb, x + 7, y + 19, 2, 1, RGB(220, 150, 130));
    fill_rect(fb, x + 23, y + 19, 2, 1, RGB(220, 150, 130));
    fill_rect(fb, x + 13, y + 21, 6, 1, RGB(150, 80, 70));

    fill_rect(fb, x + 5, y + 24, 22, 6, L->shirt);
    if (id != 0) fill_rect(fb, x + 9, y + 24, 14, 4, L->accent);
}

static void draw_dialogue(uint16_t *fb)
{
    if (!dlg_open) return;
    fill_rect(fb, 8, 148, 304, 84, C_HUD);
    fill_rect(fb, 8, 148, 304, 1, C_HUD_L);
    draw_face(fb, 16, 152, dlg_npc);
    draw_text_big_shadow(fb, 56, 154, npc_def[dlg_npc].name, C_GOLD);
    draw_hearts(fb, 220, 154, npc_points[dlg_npc]);
    draw_text_big_shadow(fb, 56, 174, dlg_lines[dlg_page], C_WHITE);
    draw_text_shadow(fb, 16, 214, "A next   B close", C_WHITE);
}

static const char *npc_where(int i)
{
    const Sched *s = npc_sched(i);
    while (s[1].start != 1440 && s[1].start <= (uint16_t)minutes) s++;
    if (s->scene == SC_TOWN) return "town";
    if (s->scene == SC_SHOP) return "shop";
    return "home";
}

static void draw_status(uint16_t *fb)
{
    int x = 16, y = 44, w = 288, h = 184;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);

    if (status_page == 0) {
        draw_text_big_shadow(fb, x + 16, y + 8, "farm book", C_GOLD);
        for (int i = 0; i < 3; i++) {
            int yy = y + 36 + i * 20;
            char mk = story_marker(i);
            draw_text_big_shadow(fb, x + 16, yy, npc_def[i].name, mk ? C_GOLD : C_WHITE);
            draw_hearts(fb, x + 88, yy, npc_points[i]);
            draw_text_big_shadow(fb, x + 190, yy, npc_where(i), C_GOLD);
        }
        for (int i = 0; i < 2; i++) {
            if (!(pet_owned & (1 << i))) continue;
            int yy = y + 100 + i * 20;
            draw_text_big_shadow(fb, x + 16, yy, pet_name[i + 1], C_WHITE);
            draw_hearts(fb, x + 88, yy, pet_aff[i]);
        }
        if (dino_unlocked) {
            int yy = y + 144;
            draw_text_big_shadow(fb, x + 16, yy, "dino", C_GOLD);
            draw_hearts(fb, x + 88, yy, 100);
        }
    } else if (status_page == 1) {
        draw_text_big_shadow(fb, x + 16, y + 8, "crops", C_GOLD);
        int count = season_crop_count[season];
        for (int i = 0; i < count; i++) {
            int id = season * 4 + i;
            int yy = y + 34 + i * 22;
            draw_text_big_shadow(fb, x + 16, yy, crops[id].name, C_WHITE);
            draw_num_big_shadow(fb, x + 112, yy, crops[id].days, C_WHITE);
            draw_text_big_shadow(fb, x + 124, yy, "d", C_WHITE);
            draw_num_big_shadow(fb, x + 152, yy, crops[id].sell, C_GOLD);
            draw_text_big_shadow(fb, x + 176, yy, "g", C_GOLD);
            if (crops[id].regrow) {
                draw_text_big_shadow(fb, x + 200, yy, "regrow", C_GOLD);
            }
        }
        draw_text_big_shadow(fb, x + 16, y + 124, "water", C_WHITE);
        draw_num_big_shadow(fb, x + 100, y + 124, water_can, C_WATER_L);
        draw_text_big_shadow(fb, x + 160, y + 124, "energy", C_WHITE);
        draw_num_big_shadow(fb, x + 256, y + 124, energy, C_ENERGY);
    } else if (status_page == 2) {
        draw_text_big_shadow(fb, x + 16, y + 8, "stories", C_GOLD);
        for (int i = 0; i < 3; i++) {
            int yy = y + 34 + i * 36;
            char mark = story_marker(i);
            draw_text_big_shadow(fb, x + 16, yy, npc_def[i].name,
                                 mark ? C_GOLD : C_WHITE);
            if (mark) {
                char m[2] = { mark, 0 };
                draw_text_big_shadow(fb, x + 82, yy, m, C_GOLD);
            }
            char desc[24];
            story_desc(i, desc);
            draw_text_big_shadow(fb, x + 104, yy, desc, C_WHITE);
        }
    } else {
        draw_text_big_shadow(fb, x + 16, y + 8, "upgrade", C_GOLD);
        draw_text_big_shadow(fb, x + 16, y + 32, "coins", C_WHITE);
        draw_gold(fb, x + 104, y + 32, coins);

        for (int i = 0; i < 2; i++) {
            int yy = y + 64 + i * 30;
            if (i == status_upg_sel) fill_rect(fb, x + 8, yy - 4, w - 16, 24, C_HUD_L);
            if (i == 0) {
                draw_text_big_shadow(fb, x + 16, yy, "house", C_WHITE);
                draw_text_big_shadow(fb, x + 100, yy, "lv", C_WHITE);
                draw_num_big_shadow(fb, x + 136, yy, house_level, C_GOLD);
                if (house_level >= 4) {
                    draw_text_big_shadow(fb, x + 208, yy, "max", C_GOLD);
                } else {
                    draw_gold(fb, x + 196, yy, house_cost[house_level]);
                }
            } else {
                draw_text_big_shadow(fb, x + 16, yy, "can", C_WHITE);
                draw_text_big_shadow(fb, x + 76, yy, "cap", C_WHITE);
                draw_num_big_shadow(fb, x + 124, yy, water_can_max, C_WATER_L);
                if (can_level >= 2) {
                    draw_text_big_shadow(fb, x + 208, yy, "max", C_GOLD);
                } else {
                    draw_gold(fb, x + 196, yy, can_cost[can_level]);
                }
            }
        }

        draw_text_big_shadow(fb, x + 16, y + 142, "energy", C_WHITE);
        draw_num_big_shadow(fb, x + 100, y + 142, energy_max, C_ENERGY);
    }

    if (status_page == 3) {
        draw_text_big_shadow(fb, x + 16, y + h - 18, "A buy", C_WHITE);
    } else {
        draw_text_big_shadow(fb, x + 16, y + h - 18, "A next", C_WHITE);
    }
    draw_text_big_shadow(fb, x + 140, y + h - 18, "B close", C_WHITE);
}

static void draw_bin(uint16_t *fb)
{
    int x = 16, y = 44, w = 288, h = 184;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);

    int total = 0;
    for (int i = 0; i < 16; i++) total += produce[i] * crops[i].sell;
    for (int i = 0; i < 4; i++) total += fish[i] * fish_price[i];

    draw_text_big_shadow(fb, x + 16, y + 8, "shipping bin", C_GOLD);
    int tn = 1, tv = total;
    while (tv >= 10) { tn++; tv /= 10; }
    draw_num_big_shadow(fb, x + w - 24 - tn * 12, y + 8, total, C_GOLD);
    draw_text_big_shadow(fb, x + w - 22, y + 8, "g", C_GOLD);

    int row = 0;
    for (int i = 0; i < 16 && row < 4; i++) {
        if (!produce[i]) continue;
        int ry = y + 38 + row * 22;
        draw_text_big_shadow(fb, x + 16, ry, crops[i].name, C_WHITE);
        draw_text_big_shadow(fb, x + 132, ry, "x", C_WHITE);
        draw_num_big_shadow(fb, x + 152, ry, produce[i], C_WHITE);
        draw_text_big_shadow(fb, x + 192, ry, "+", C_GOLD);
        draw_num_big_shadow(fb, x + 208, ry, produce[i] * crops[i].sell, C_GOLD);
        row++;
    }
    for (int i = 0; i < 4 && row < 4; i++) {
        if (!fish[i]) continue;
        int ry = y + 38 + row * 22;
        draw_text_big_shadow(fb, x + 16, ry, fish_name[i], C_WHITE);
        draw_text_big_shadow(fb, x + 132, ry, "x", C_WHITE);
        draw_num_big_shadow(fb, x + 152, ry, fish[i], C_WHITE);
        draw_text_big_shadow(fb, x + 192, ry, "+", C_GOLD);
        draw_num_big_shadow(fb, x + 208, ry, fish[i] * fish_price[i], C_GOLD);
        row++;
    }

    draw_text_big_shadow(fb, x + 16, y + 150, "A ship", C_WHITE);
    draw_text_big_shadow(fb, x + 136, y + 150, "B close", C_WHITE);
}

static void draw_summary(uint16_t *fb)
{
    int x = 16, y = 44, w = 288, h = 184;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 16, y + 8, "sold", C_GOLD);

    int row = 0;
    for (int i = 0; i < 16 && row < 4; i++) {
        if (!summary_items[i]) continue;
        int ry = y + 30 + row * 20;
        draw_text_big_shadow(fb, x + 16, ry, crops[i].name, C_WHITE);
        draw_text_big_shadow(fb, x + 132, ry, "x", C_WHITE);
        draw_num_big_shadow(fb, x + 152, ry, summary_items[i], C_WHITE);
        draw_text_big_shadow(fb, x + 192, ry, "+", C_GOLD);
        draw_num_big_shadow(fb, x + 208, ry, summary_items[i] * crops[i].sell, C_GOLD);
        row++;
    }
    for (int i = 0; i < 4 && row < 4; i++) {
        if (!summary_fish[i]) continue;
        int ry = y + 30 + row * 20;
        draw_text_big_shadow(fb, x + 16, ry, fish_name[i], C_WHITE);
        draw_text_big_shadow(fb, x + 132, ry, "x", C_WHITE);
        draw_num_big_shadow(fb, x + 152, ry, summary_fish[i], C_WHITE);
        draw_text_big_shadow(fb, x + 192, ry, "+", C_GOLD);
        draw_num_big_shadow(fb, x + 208, ry, summary_fish[i] * fish_price[i], C_GOLD);
        row++;
    }

    draw_text_big_shadow(fb, x + 16, y + 116, "total", C_WHITE);
    draw_num_big_shadow(fb, x + 100, y + 116, summary_gold, C_GOLD);
    draw_text_big_shadow(fb, x + 168, y + 116, "g", C_GOLD);
    draw_text_big_shadow(fb, x + 16, y + 140, "coins", C_WHITE);
    draw_num_big_shadow(fb, x + 100, y + 140, coins, C_GOLD);
    draw_text_big_shadow(fb, x + 168, y + 140, "g", C_GOLD);
    draw_text_big_shadow(fb, x + 16, y + h - 18, "A ok", C_WHITE);
}

static void draw_menu_main(uint16_t *fb)
{
    int x = 72, y = 40, w = 176, h = 194;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 16, y + 8, "menu", C_GOLD);

    static const char *opts[7] = {"resume", "bag", "hearts", "house", "save", "load", "sleep"};
    for (int i = 0; i < 7; i++) {
        int yy = y + 30 + i * 18;
        if (i == menu_sel) fill_rect(fb, x + 8, yy - 4, w - 16, 20, C_HUD_L);
        uint16_t col = C_WHITE;
        if (i == 3 && scene_id != SC_HOME) col = RGB(130, 130, 130);
        if (i == 6 && !player_near_bed()) col = RGB(130, 130, 130);
        draw_text_big_shadow(fb, x + 16, yy, opts[i], col);
    }
    draw_text_big_shadow(fb, x + 8, y + h - 20, "A ok  B close", C_WHITE);
}


static void draw_fish_icon(uint16_t *fb, int x, int y, int type)
{
    static const uint16_t body[4] = {
        RGB(192, 206, 214), RGB(96, 144, 88), RGB(226, 132, 132), RGB(64, 104, 164)
    };
    static const uint16_t fin[4] = {
        RGB(160, 180, 196), RGB(70, 112, 70), RGB(196, 96, 110), RGB(236, 196, 72)
    };
    uint16_t c = body[type], f = fin[type];
    fill_rect(fb, x + 2, y + 5, 11, 7, RGB(36, 32, 30));
    if (type == 0) {
        fill_rect(fb, x + 3, y + 6, 8, 4, c);
        fill_rect(fb, x + 11, y + 5, 3, 2, f);
        fill_rect(fb, x + 11, y + 8, 3, 2, f);
        fill_rect(fb, x + 6, y + 4, 3, 1, f);
        put_px(fb, x + 5, y + 6, C_WHITE);
        put_px(fb, x + 6, y + 6, RGB(30, 30, 30));
    } else if (type == 1) {
        fill_rect(fb, x + 3, y + 5, 9, 7, c);
        fill_rect(fb, x + 10, y + 4, 3, 3, c);
        fill_rect(fb, x + 2, y + 6, 2, 3, RGB(180, 206, 150));
        fill_rect(fb, x + 6, y + 3, 3, 2, f);
        fill_rect(fb, x + 5, y + 7, 3, 2, RGB(200, 224, 170));
        put_px(fb, x + 9, y + 6, RGB(30, 30, 30));
        put_px(fb, x + 4, y + 6, C_WHITE);
    } else if (type == 2) {
        fill_rect(fb, x + 3, y + 5, 9, 6, c);
        fill_rect(fb, x + 11, y + 4, 3, 2, f);
        fill_rect(fb, x + 11, y + 9, 3, 2, f);
        fill_rect(fb, x + 4, y + 4, 5, 2, RGB(244, 170, 150));
        fill_rect(fb, x + 5, y + 6, 1, 1, RGB(120, 48, 48));
        fill_rect(fb, x + 8, y + 7, 1, 1, RGB(120, 48, 48));
        put_px(fb, x + 9, y + 6, RGB(30, 30, 30));
        put_px(fb, x + 4, y + 8, RGB(255, 220, 200));
    } else {
        fill_rect(fb, x + 2, y + 4, 11, 8, c);
        fill_rect(fb, x + 11, y + 3, 3, 3, f);
        fill_rect(fb, x + 11, y + 9, 3, 3, f);
        fill_rect(fb, x + 5, y + 3, 4, 2, f);
        fill_rect(fb, x + 4, y + 6, 6, 3, RGB(120, 156, 206));
        put_px(fb, x + 10, y + 6, RGB(20, 26, 36));
        put_px(fb, x + 4, y + 5, C_WHITE);
    }
}
static void draw_tool_icon(uint16_t *fb, int x, int y, int kind)
{
    if (kind == IT_HOE) {
        fill_rect(fb, x + 3, y + 2, 2, 10, RGB(150, 104, 62));
        fill_rect(fb, x + 4, y + 9, 2, 3, RGB(130, 88, 52));
        fill_rect(fb, x + 9, y + 2, 5, 2, RGB(180, 184, 190));
        fill_rect(fb, x + 9, y + 3, 2, 2, RGB(130, 134, 140));
    } else if (kind == IT_CAN) {
        fill_rect(fb, x + 3, y + 5, 8, 7, RGB(90, 150, 190));
        fill_rect(fb, x + 4, y + 6, 6, 2, RGB(150, 200, 224));
        fill_rect(fb, x + 10, y + 4, 3, 2, RGB(90, 150, 190));
        fill_rect(fb, x + 12, y + 3, 2, 2, RGB(70, 120, 160));
        fill_rect(fb, x + 5, y + 3, 4, 2, RGB(70, 120, 160));
    } else if (kind == IT_ROD) {
        fill_rect(fb, x + 3, y + 12, 2, 2, RGB(150, 104, 62));
        fill_rect(fb, x + 4, y + 4, 2, 8, RGB(150, 104, 62));
        fill_rect(fb, x + 5, y + 2, 2, 3, RGB(150, 104, 62));
        fill_rect(fb, x + 7, y + 2, 6, 1, RGB(220, 210, 180));
        put_px(fb, x + 13, y + 3, RGB(220, 210, 180));
        fill_rect(fb, x + 12, y + 4, 2, 2, RGB(180, 180, 180));
        put_px(fb, x + 12, y + 6, C_ROSE);
    } else {
        fill_rect(fb, x + 7, y + 1, 3, 9, RGB(210, 214, 220));
        fill_rect(fb, x + 8, y + 2, 1, 6, C_WHITE);
        fill_rect(fb, x + 5, y + 10, 7, 2, C_GOLD);
        fill_rect(fb, x + 7, y + 12, 3, 2, RGB(120, 72, 42));
    }
}

static int bag_get(int idx, int *kind, int *id, int *count)
{
    if (idx < 0 || idx >= 30) return 0;
    if (idx == 0) { *kind = IT_HOE; *id = 0; *count = 1; return 1; }
    if (idx == 1) { *kind = IT_CAN; *id = 0; *count = 1; return 1; }
    if (idx == 2) {
        if (!rod_owned) return 0;
        *kind = IT_ROD; *id = 0; *count = 1; return 1;
    }
    if (idx == 3) {
        if (!(story[0] >= 4 && story[1] >= 4 && story[2] >= 4)) return 0;
        *kind = IT_SWORD; *id = 0; *count = 1; return 1;
    }
    int base = 4;
    int n = season_crop_count[season];
    if (idx < base + n) {
        int cid = season * 4 + (idx - base);
        if (!seeds[cid]) return 0;
        *kind = IT_SEED; *id = cid; *count = seeds[cid]; return 1;
    }
    base += n;
    if (idx < base + 16) {
        int cid = idx - base;
        if (!produce[cid]) return 0;
        *kind = IT_CROP; *id = cid; *count = produce[cid]; return 1;
    }
    base += 16;
    if (idx < base + 4) {
        int fid = idx - base;
        if (!fish[fid]) return 0;
        *kind = IT_FISH; *id = fid; *count = fish[fid]; return 1;
    }
    return 0;
}

static int bag_find_selected(void)
{
    if (sel_kind == IT_HOE) return 0;
    if (sel_kind == IT_CAN) return 1;
    if (sel_kind == IT_ROD) return 2;
    if (sel_kind == IT_SWORD) return 3;
    if (sel_kind == IT_SEED) {
        if (sel_id / 4 == season) return 4 + (sel_id & 3);
        return 0;
    }
    int n = season_crop_count[season];
    if (sel_kind == IT_CROP) return 4 + n + sel_id;
    if (sel_kind == IT_FISH) return 4 + n + 16 + sel_id;
    return 0;
}

static void draw_bag(uint16_t *fb)
{
    int x = 16, y = 44, w = 288, h = 184;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 16, y + 8, "bag", C_GOLD);
    draw_text_shadow(fb, x + 200, y + 14, "A select  B close", C_WHITE);

    for (int i = 0; i < 30; i++) {
        int cx = x + 8 + (i % 6) * 44;
        int cy = y + 28 + (i / 6) * 24;
        fill_rect(fb, cx, cy, 42, 22, RGB(70, 52, 38));
        fill_rect(fb, cx, cy, 42, 1, RGB(96, 74, 52));
        if (i == bag_sel) {
            fill_rect(fb, cx, cy, 42, 1, C_GOLD);
            fill_rect(fb, cx, cy + 21, 42, 1, C_GOLD);
            fill_rect(fb, cx, cy, 1, 22, C_GOLD);
            fill_rect(fb, cx + 41, cy, 1, 22, C_GOLD);
        }
        int kind, id, count;
        if (bag_get(i, &kind, &id, &count)) {
            if (kind == IT_HOE || kind == IT_CAN || kind == IT_ROD || kind == IT_SWORD) {
                draw_tool_icon(fb, cx + 5, cy + 3, kind);
            } else if (kind == IT_FISH) {
                draw_fish_icon(fb, cx + 5, cy + 3, id);
            } else {
                draw_item_icon(fb, cx + 5, cy + 3, id, kind == IT_SEED);
            }
            if (kind == IT_SEED || kind == IT_CROP || kind == IT_FISH) {
                char num[6];
                int n = 0, v = count;
                num[n++] = 'x';
                if (v >= 10) num[n++] = (char)('0' + v / 10);
                num[n++] = (char)('0' + v % 10);
                num[n] = 0;
                draw_text_shadow(fb, cx + 24, cy + 14, num, C_WHITE);
            }
            if (kind == sel_kind && id == sel_id) {
                fill_rect(fb, cx + 1, cy + 1, 2, 2, C_ENERGY);
            }
        }
    }

    int kind, id, count;
    if (bag_get(bag_sel, &kind, &id, &count)) {
        draw_text_big_shadow(fb, x + 16, y + 148, item_name_full(kind, id), C_WHITE);
        if (kind == IT_SEED || kind == IT_CROP || kind == IT_FISH) {
            draw_num_big_shadow(fb, x + 170, y + 148, count, C_GOLD);
        }
    } else {
        draw_text_big_shadow(fb, x + 16, y + 148, "empty", C_WHITE);
    }
    draw_text_shadow(fb, x + 16, y + h - 18, "A select   B close", C_WHITE);
}

static void draw_menu(uint16_t *fb)
{
    if (menu_page == 1) {
        draw_bag(fb);
        return;
    }
    if (menu_page == 2) {
        int x = 60, y = 80, w = 200, h = 80;
        fill_rect(fb, x, y, w, h, C_HUD);
        fill_rect(fb, x, y, w, 1, C_HUD_L);
        fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);
        draw_text_big_shadow(fb, x + 16, y + 12, "load save?", C_GOLD);
        draw_text_big_shadow(fb, x + 16, y + 42, "A yes  B no", C_WHITE);
        return;
    }
    draw_menu_main(fb);
}

static void draw_prompt(uint16_t *fb)
{
    static char prompt_buf[40];
    if (message_timer > 0 || dlg_open || shop_open || status_open || bin_open || summary_open || menu_open) return;

    const char *s = 0;
    int n = npc_near();
    if (n >= 0) {
        s = "A talk";
        if (!npc_gifted[n] && (sel_kind == IT_CROP || sel_kind == IT_FISH) && sel_count() > 0) {
            s = "A talk  B gift";
        }
    } else if (shell_near() >= 0) {
        s = "A shell";
    } else if (door_adjacent()) {
        const Door *dd = door_adjacent();
        s = (dd && dd->target == SC_MINE && !mine_unlocked()) ? "A locked" : "A enter";
    } else if (player_near_bed()) {
        s = "A sleep";
    } else {
        int tx, ty;
        target_tile(&tx, &ty);
        uint8_t t = map_get(tx, ty);
        int pi = (scene_id == SC_FARM) ? plot_index(tx, ty) : -1;
        if (pi >= 0) {
            if (plot_crop[pi] == 0) {
                s = (sel_kind == IT_SEED && sel_id / 4 == season) ? "A plant" : "select seed";
            } else {
                int cid = plot_crop[pi] - 1;
                const char *verb = "harvest ";
                if (plot_growth[pi] < crops[cid].days) {
                    if (plot_water[pi]) verb = "wet ";
                    else verb = (sel_kind == IT_CAN) ? "water " : "select can";
                }
                int p = 0;
                prompt_buf[p++] = 'A';
                prompt_buf[p++] = ' ';
                for (const char *v = verb; *v && p < 20; v++) prompt_buf[p++] = *v;
                const char *nm = crops[cid].name;
                while (*nm && p < 34) prompt_buf[p++] = *nm++;
                prompt_buf[p] = 0;
                s = prompt_buf;
            }
        } else if (scene_id == SC_SHOP && t == T_SIGN) s = "A pet";
        else if (t == T_BOWL || t == T_BOWL2) {
            int pi = (t == T_BOWL) ? 0 : 1;
            s = (pet_owned & (1 << pi)) ? "A feed" : "A bowl";
        }
        else if (scene_id == SC_BEACH && t == T_SIGN) s = "A rod";
        else if (scene_id == SC_BEACH && (t == T_WATER || t == T_PIER))
            s = (rod_owned && sel_kind == IT_ROD) ? "A fish" : "select rod";
        else if (t == T_STAND || t == T_COUNTER) s = "A shop";
        else if (t == T_BED) s = "A sleep";
        else if (t == T_TABLE) s = "A book";
        else if (t == T_BIN) s = "A bin";
        else if (t == T_MAILBOX) s = "A mail";
        else if (t == T_FOUNTAIN) s = (all_stories_done() && !(secrets & SEC_CAMPFIRE) && minutes >= 18 * 60) ? "A talk" : "A wish";
        else if (t == T_WATER) s = (sel_kind == IT_CAN) ? "A refill" : "select can";
        if (scene_id == SC_MINE) s = (sel_kind == IT_SWORD) ? "A swing" : "select sword";
    }
    if (!s) return;

    int len = 0;
    while (s[len]) len++;
    int w = len * 12 + 16;
    int x = (GAME_W - w) / 2;
    int y = 212;
    fill_rect(fb, x, y, w, 24, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 8, y + 4, s, C_WHITE);
}

static void draw_title(uint16_t *fb, int t)
{
    /* The intro is fully procedural: no sprite sheet or scratch buffer is
       retained after it finishes. */
    if (t > TITLE_END) t = TITLE_END;
    int dawn = t < 90 ? t : 90;
    int horizon = 156;

    for (int y = 0; y < horizon; y++) {
        int k = dawn + y / 3;
        if (k > 90) k = 90;
        int r = 18 + k * 2;
        int g = 30 + k;
        int b = 54 + k / 2;
        if (r > 238) r = 238;
        if (g > 174) g = 174;
        if (b > 116) b = 116;
        fill_rect(fb, 0, y, GAME_W, 1, RGB(r, g, b));
    }

    static const uint16_t stars[][2] = {
        {20,18},{54,36},{92,12},{128,30},{166,16},{206,38},{244,14},{286,34},
        {38,58},{112,54},{184,58},{262,52},{74,74},{220,72},{300,64}
    };
    if (t < 70) {
        for (unsigned i = 0; i < sizeof(stars) / sizeof(stars[0]); i++) {
            if ((t + i * 7) % 30 < 24) put_px(fb, stars[i][0], stars[i][1], RGB(240, 238, 214));
        }
    }

    int sun_y = 138 - (t < 80 ? t : 80);
    int sr = 15;
    for (int dy = -sr; dy <= sr; dy++) {
        int w = sr * sr - dy * dy;
        int ww = 1;
        while ((ww + 1) * (ww + 1) <= w) ww++;
        fill_rect(fb, 252 - ww, sun_y + dy, ww * 2, 1, RGB(250, 184, 92));
    }
    fill_rect(fb, 246, sun_y - 9, 12, 2, RGB(255, 214, 130));

    for (int x = 0; x < GAME_W; x++) {
        int h = 18 + (x * 7 + 11) % 19;
        fill_rect(fb, x, horizon - h, 1, h, RGB(44, 82, 58));
    }
    fill_rect(fb, 0, horizon, GAME_W, GAME_H - horizon, RGB(62, 118, 66));
    fill_rect(fb, 0, horizon + 12, GAME_W, 2, RGB(48, 92, 54));

    /* Little farmhouse. */
    fill_rect(fb, 38, 122, 48, 26, RGB(126, 86, 54));
    fill_rect(fb, 40, 124, 44, 22, RGB(196, 154, 102));
    for (int i = 0; i < 30; i++) {
        fill_rect(fb, 34 + i, 112 - i / 2, 56 - i * 2, 2, RGB(150, 62, 48));
    }
    fill_rect(fb, 58, 132, 10, 14, RGB(86, 56, 34));
    fill_rect(fb, 48, 128, 6, 6, RGB(92, 154, 176));
    fill_rect(fb, 72, 128, 6, 6, RGB(92, 154, 176));

    /* Tree. */
    fill_rect(fb, 268, 128, 6, 22, RGB(96, 62, 38));
    fill_rect(fb, 250, 104, 42, 24, RGB(34, 78, 44));
    fill_rect(fb, 256, 96, 30, 14, RGB(48, 104, 54));
    fill_rect(fb, 246, 112, 12, 10, RGB(48, 104, 54));
    fill_rect(fb, 286, 114, 10, 8, RGB(48, 104, 54));
    fill_rect(fb, 258, 100, 12, 4, RGB(78, 142, 70));

    /* Fence and crop rows. */
    for (int x = 22; x < 220; x += 18) {
        fill_rect(fb, x, 166, 2, 22, RGB(150, 104, 62));
        fill_rect(fb, x, 172, 16, 2, RGB(176, 128, 76));
    }
    for (int x = 26; x < 210; x += 26) {
        fill_rect(fb, x, 188, 3, 8, RGB(64, 120, 58));
        fill_rect(fb, x - 3, 186, 9, 3, RGB(84, 146, 70));
    }

    /* Pets at the edge of the field. */
    if (t > 110) {
        fill_rect(fb, 224, 186, 10, 7, RGB(210, 142, 72));
        fill_rect(fb, 227, 182, 7, 5, RGB(224, 156, 84));
        put_px(fb, 230, 184, RGB(30, 26, 22));
        fill_rect(fb, 294, 184, 9, 8, C_WHITE);
        fill_rect(fb, 298, 180, 5, 5, C_WHITE);
        fill_rect(fb, 300, 178, 2, 3, RGB(214, 58, 52));
    }

    /* Wooden sign / logo. */
    if (t >= 70) {
        int y = 52 + (t < 110 ? (110 - t) / 2 : 0);
        int x = 42, w = 236, h = 104;
        fill_rect(fb, x, y, w, h, C_GOLD);
        fill_rect(fb, x + 2, y + 2, w - 4, h - 4, C_HUD);
        fill_rect(fb, x + 4, y + 4, w - 8, 1, C_HUD_L);
        for (int yy = y + 12; yy < y + h - 8; yy += 14) {
            fill_rect(fb, x + 6, yy, w - 12, 1, RGB(68, 48, 34));
        }
        put_px(fb, x + 8, y + 8, C_GOLD);
        put_px(fb, x + w - 9, y + 8, C_GOLD);
        put_px(fb, x + 8, y + h - 9, C_GOLD);
        put_px(fb, x + w - 9, y + h - 9, C_GOLD);
        draw_text_big_shadow(fb, 100, y + 16, "mini farm", C_GOLD);
        draw_item_icon(fb, 64, y + 18, 0, 1);
        draw_shell(fb, x + w - 76, y + 18);
        draw_text_big_shadow(fb, 82, y + 52, "a little life", C_WHITE);
        if (t >= TITLE_END) {
            draw_text_big_shadow(fb, 116, y + 78, "press A", C_WHITE);
        }
    }

    if (t == TITLE_END) {
        put_px(fb, 8, 8, RGB(120, 140, 120));
    }
}

static void draw_ending(uint16_t *fb)
{
    fill_rect(fb, 0, 0, GAME_W, GAME_H, C_BLACK);
    if (ending == 3) draw_text_big_shadow(fb, 82, 56, "best friends!", C_GOLD);
    else draw_text_big_shadow(fb, 106, 56, "year one!", C_GOLD);
    draw_text_big_shadow(fb, 46, 96, "the town is smiling", C_WHITE);
    draw_text_big_shadow(fb, 40, 128, "3 stories  3 friends", C_WHITE);
    if (ending == 3) draw_text_big_shadow(fb, 82, 160, "dino too!", C_GOLD);
    draw_text_big_shadow(fb, 116, 200, "press A", C_WHITE);
}

/* --- weather ----------------------------------------------------------- */

static uint32_t rng_state = 12345;

static int rnd(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return (int)((rng_state >> 16) & 0x7fff);
}

static int weather_next(void)
{
    if (season == 3) return (rnd() % 100) < 25 ? 2 : 0;
    return (rnd() % 100) < 22 ? 1 : 0;
}

static void draw_weather(uint16_t *fb)
{
    if (weather == 1) {
        for (int i = 0; i < 90; i++) {
            int x = (i * 37 + (int)(water_anim / 2) * 3) % GAME_W;
            int y = (i * 53 + (int)(water_anim / 2) * 5) % GAME_H;
            put_px(fb, x, y, C_WATER_L);
            put_px(fb, x + 1, y + 2, C_WATER_L);
            put_px(fb, x + 2, y + 4, RGB(220, 244, 240));
            if (y > 12) put_px(fb, x + 1, y - 2, C_WATER_L);
        }
    } else if (weather == 2) {
        for (int i = 0; i < 80; i++) {
            int x = (i * 41 + (int)(water_anim / 3) * 2) % GAME_W;
            int y = (i * 59 + (int)(water_anim / 3) * 3) % GAME_H;
            put_px(fb, x, y, C_WHITE);
            put_px(fb, x + 1, y, C_WHITE);
            put_px(fb, x, y + 1, RGB(226, 236, 244));
        }
    }
}

/* --- actions ----------------------------------------------------------- */

static void advance_minutes(int n)
{
    minutes += n;
    if (minutes > 1439) minutes = 1439;
}

static void set_message(const char *m)
{
    message = m;
    message_timer = 60;
}

static void set_message_plus(const char *name)
{
    char *p = msg_buf;
    *p++ = '+';
    *p++ = '1';
    *p++ = ' ';
    while (*name && p < msg_buf + 22) *p++ = *name++;
    *p = 0;
    set_message(msg_buf);
}

static void target_tile(int *tx, int *ty)
{
    int px = (player_x + 8) / TILE;
    int py = (player_y + 12) / TILE;
    uint8_t under = map_get(px, py);

    if (under == T_SOIL || under == T_SOIL_W) {
        *tx = px;
        *ty = py;
        return;
    }

    *tx = px;
    *ty = py;
    if (facing == 0) (*ty)++;
    else if (facing == 1) (*ty)--;
    else if (facing == 2) (*tx)--;
    else (*tx)++;
}

static void start_fade(const Door *d)
{
    if (fade_timer > 0) return;
    if (d->target == SC_MINE && !mine_unlocked()) {
        set_message("mine locked");
        return;
    }
    fade_timer = 20;
    sound_play(SFX_DOOR);
    fade_scene = d->target;
    fade_x = d->tx * TILE;
    fade_y = d->ty * TILE;
    fade_dir = d->dir;
}

static void open_shop(void)
{
    shop_open = 1;
    shop_sel = 0;
    shop_qty = 1;
}

static void deposit_bin(void)
{
    int total = 0;
    for (int i = 0; i < 16; i++) {
        if (produce[i]) {
            bin_items[i] += produce[i];
            total += produce[i] * crops[i].sell;
            produce[i] = 0;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (fish[i]) {
            bin_fish[i] += fish[i];
            total += fish[i] * fish_price[i];
            fish[i] = 0;
        }
    }
    bin_value += total;
    bin_open = 0;
    advance_minutes(2);
    if (total) sound_play(SFX_COIN);
    set_message(total ? "in bin" : "bin empty");
}

static void open_bin(void)
{
    int any = 0;
    for (int i = 0; i < 16; i++) if (produce[i]) any = 1;
    for (int i = 0; i < 4; i++) if (fish[i]) any = 1;
    if (!any) {
        set_message("bin empty");
        return;
    }
    bin_open = 1;
}

static int pick_fish(void)
{
    int r = rnd() % 100;
    if (season == 3) return r < 70 ? 2 : 1;
    if (season == 1 && r < 35) return 3;
    if (weather == 1 && r < 45) return 2;
    return r < 70 ? 0 : 1;
}

static void fish_message(void)
{
    char *p = fish_msg;
    *p++ = '+';
    *p++ = '1';
    *p++ = ' ';
    const char *nm = fish_name[fish_type];
    while (*nm && p < fish_msg + 22) *p++ = *nm++;
    *p = 0;
}

static void start_fishing(void)
{
    if (!rod_owned) {
        set_message("need rod");
        return;
    }
    if (energy < 2) {
        set_message("too tired");
        return;
    }
    energy -= 2;
    advance_minutes(10);
    fish_type = pick_fish();
    fish_state = FISH_WAIT;
    fish_timer = 60 + rnd() % 120;
    sound_play(SFX_CLICK);
    set_message("cast");
}

static void fish_update(const Input *in, int a_edge, int b_edge)
{
    if (fish_state == FISH_WAIT) {
        if (b_edge) { fish_state = FISH_IDLE; set_message("cancel"); return; }
        if (--fish_timer <= 0) {
            fish_state = FISH_BITE;
            fish_timer = 60;
            sound_play(SFX_CLICK);
            set_message("!");
        }
        return;
    }

    if (fish_state == FISH_BITE) {
        if (b_edge) { fish_state = FISH_IDLE; set_message("cancel"); return; }
        if (a_edge) {
            fish_state = FISH_GAME;
            fish_timer = 900;
            fish_pos = 50;
            fish_vel = 2;
            fish_zone_h = 36 - fish_type * 4;
            fish_green = 50;
            fish_catch = 20;
            return;
        }
        if (--fish_timer <= 0) {
            fish_state = FISH_IDLE;
            sound_play(SFX_FAIL);
            set_message("got away");
        }
        return;
    }

    /* FISH_GAME */
    if (b_edge) { fish_state = FISH_IDLE; set_message("cancel"); return; }
    if (in->a) fish_green -= 2;
    else fish_green += 2;
    if (fish_green < 0) fish_green = 0;
    int gmax = 126 - fish_zone_h;
    if (fish_green > gmax) fish_green = gmax;

    if (--fish_timer <= 0) {
        fish_state = FISH_IDLE;
        sound_play(SFX_FAIL);
        set_message("got away");
        return;
    }

    if ((fish_timer % 12) == 0) {
        int v = rnd() % 7 - 3;
        if (v == 0) v = 2;
        fish_vel = v;
    }
    fish_pos += fish_vel;
    if (fish_pos < 0) { fish_pos = 0; fish_vel = 2; }
    if (fish_pos > 124) { fish_pos = 124; fish_vel = -2; }

    int overlap = (fish_pos + 7 >= fish_green && fish_pos <= fish_green + fish_zone_h);
    fish_catch += overlap ? 2 : -2;
    if (fish_catch < 0) fish_catch = 0;
    if (fish_catch > 100) fish_catch = 100;

    if (fish_catch >= 100) {
        if (!(secrets & SEC_GOLDEN) && minutes >= 300 && minutes < 420 && (rnd() % 100) < 2) {
            coins += 500;
            secrets |= SEC_GOLDEN;
            save_now();
            sound_play(SFX_CATCH);
            set_message("golden fish!");
            fish_state = FISH_IDLE;
            return;
        }
        if (fish[fish_type] < 15) fish[fish_type]++;
        fish_message();
        sound_play(SFX_CATCH);
        set_message(fish_msg);
        fish_state = FISH_IDLE;
    } else if (fish_catch <= 0) {
        fish_state = FISH_IDLE;
        sound_play(SFX_FAIL);
        set_message("got away");
    }
}

static int use_tool(void)
{
    int tx, ty;
    target_tile(&tx, &ty);
    if (tx < 0 || ty < 0) return 0;

    uint8_t t = map_get(tx, ty);

    if (t == T_FOUNTAIN) {
        if (all_stories_done() && !(secrets & SEC_CAMPFIRE) && minutes >= 18 * 60) {
            secrets |= SEC_CAMPFIRE;
            for (int i = 0; i < 3; i++) {
                npc_points[i] += 10;
                if (npc_points[i] > 100) npc_points[i] = 100;
            }
            save_now();
            dlg_lines[0] = "mira: we did it.";
            dlg_lines[1] = "tom: the pier is fixed.";
            dlg_lines[2] = "lily: my crown!";
            dlg_lines[3] = "all: good night!";
            dlg_npc = 0;
            dlg_page = 0;
            dlg_pages = 4;
            dlg_open = 1;
            sound_play(SFX_GIFT);
            return 1;
        }
        if (!wish_done && minutes >= 20 * 60) {
            if (coins >= 10) {
                coins -= 10;
                wish_done = 1;
                if ((rnd() % 4) == 0) {
                    wish_rain = 1;
                    set_message("rain tomorrow?");
                } else {
                    const char *w[] = {"i wish for rain.", "i wish for pie.", "i wish for a friend."};
                    set_message(w[rnd() % 3]);
                }
                sound_play(SFX_COIN);
                save_now();
            } else {
                set_message("need 10g");
            }
            return 1;
        }
        set_message("the water is cool.");
        return 1;
    }

    if (scene_id == SC_MINE && t != T_DOOR) {
        if (sel_kind != IT_SWORD) { set_message("select sword"); return 1; }
        if (t == T_ROCK || t == T_ORE) {
            scene_tiles[scene_id][ty * scene_w[scene_id] + tx] = T_MINE_FLOOR;
            if (t == T_ORE) {
                if (!(secrets & SEC_FOSSIL) && (rnd() % 100) < 2) {
                    if (produce[14] < 15) produce[14]++;
                    secrets |= SEC_FOSSIL;
                    save_now();
                    set_message(dino_unlocked ? "dino fossil!" : "fossil!");
                } else {
                    if (ore_count < 255) ore_count++;
                    set_message("+1 ore");
                }
            } else {
                set_message("rock broke");
            }
            sound_play(SFX_HARVEST);
            advance_minutes(2);
            return 1;
        }
        int hx = tx * TILE + 8;
        int hy = ty * TILE + 8;
        for (int i = 0; i < 3; i++) {
            if (!slime_alive[i]) continue;
            if (iabs(slime_x[i] + 8 - hx) < 20 && iabs(slime_y[i] + 8 - hy) < 20) {
                slime_hp[i]--;
                if (slime_hp[i] <= 0) {
                    slime_alive[i] = 0;
                    set_message("slime down");
                } else {
                    set_message("hit!");
                }
                sound_play(SFX_HARVEST);
                return 1;
            }
        }
        set_message("swing");
        return 1;
    }

    if (t == T_MAILBOX) {
        if (all_stories_done() && !(secrets & SEC_LETTER)) {
            secrets |= SEC_LETTER;
            save_now();
            dlg_lines[0] = "to the new farmer.";
            dlg_lines[1] = "the soil remembers.";
            dlg_lines[2] = "take care of mira.";
            dlg_lines[3] = "- grandma";
            dlg_npc = 0;
            dlg_page = 0;
            dlg_pages = 4;
            dlg_open = 1;
            sound_play(SFX_GIFT);
        } else {
            set_message("no mail today.");
        }
        return 1;
    }

    if (player_near_bed()) {
        if (cat_on_bed() && !cat_bed_block) {
            cat_bed_block = 1;
            set_message("cat is sleeping.");
            sound_play(SFX_PET);
            return 1;
        }
        sleep_night();
        return 1;
    }

    const Door *d = door_adjacent();
    if (!d) d = door_at(scene_id, tx, ty);
    if (d) {
        start_fade(d);
        return 1;
    }

    if (scene_id == SC_SHOP && t == T_SIGN) {
        pet_shop_open = 1;
        pet_shop_sel = 0;
        sound_play(SFX_CLICK);
        return 1;
    }

    if (t == T_BOWL || t == T_BOWL2) {
        int pi = (t == T_BOWL) ? 0 : 1;
        if (!(pet_owned & (1 << pi))) {
            sound_play(SFX_ERROR);
            set_message("no pet");
        } else if (pet_fed & (1 << pi)) {
            sound_play(SFX_ERROR);
            set_message("fed today");
        } else {
            pet_fed |= (uint8_t)(1 << pi);
            pet_aff[pi] = (uint8_t)(pet_aff[pi] + 10);
            if (pet_aff[pi] > 100) pet_aff[pi] = 100;
            save_now();
            sound_play(SFX_PET);
            set_message("pet fed");
        }
        return 1;
    }

    if (scene_id == SC_BEACH && t == T_SIGN) {
        if (rod_owned) {
            sound_play(SFX_CLICK);
            set_message("rod ready");
        } else if (coins >= 150) {
            coins -= 150;
            rod_owned = 1;
            save_now();
            sound_play(SFX_COIN);
            set_message("rod bought");
        } else {
            sound_play(SFX_ERROR);
            set_message("not enough");
        }
        return 1;
    }

    if (t == T_STAND || t == T_COUNTER) {
        sound_play(SFX_CLICK);
        open_shop();
        return 1;
    }

    if (t == T_BIN) {
        sound_play(SFX_CLICK);
        open_bin();
        return 1;
    }

    if (t == T_BED) {
        sleep_night();
        return 1;
    }

    if (t == T_TABLE) {
        sound_play(SFX_CLICK);
        status_open = 1;
        status_page = 0;
        status_upg_sel = 0;
        return 1;
    }

    if (scene_id == SC_BEACH && t == T_PIER) {
        if (!dolphin_done && minutes >= 18 * 60 + 30 && minutes <= 19 * 60 + 30 &&
            player_x / TILE >= 30) {
            dolphin_done = 1;
            dolphin_timer = 120;
            set_message("a dolphin!");
            sound_play(SFX_CATCH);
            return 1;
        }
        if (sel_kind != IT_ROD) { set_message("select rod"); return 1; }
        start_fishing();
        return 1;
    }

    if (t == T_WATER) {
        if (scene_id == SC_BEACH) {
            if (sel_kind != IT_ROD) { set_message("select rod"); return 1; }
            start_fishing();
            return 1;
        }
        if (sel_kind != IT_CAN) { set_message("select can"); return 1; }
        water_can = water_can_max;
        advance_minutes(2);
        sound_play(SFX_WATER);
        set_message("refilled");
        return 1;
    }

    if (t == T_GRASS || t == T_GRASS2) {
        if (sel_kind == IT_HOE) set_message("soil is ready");
        else if (sel_kind == IT_SWORD) set_message("mine only");
        else set_message("not here");
        return 1;
    }

    if (t == T_SOIL || t == T_SOIL_W) {
        int idx = plot_index(tx, ty);
        if (idx < 0) { set_message("not here"); return 1; }

        if (plot_crop[idx] == 0) {
            if (sel_kind != IT_SEED) { sound_play(SFX_ERROR); set_message("select seed"); return 1; }
            if (sel_id / 4 != season) { sound_play(SFX_ERROR); set_message("not in season"); return 1; }
            if (seeds[sel_id] == 0) { sound_play(SFX_ERROR); set_message("no seeds"); return 1; }
            if (energy <= 0) { sound_play(SFX_ERROR); set_message("too tired"); return 1; }
            plot_crop[idx] = (uint8_t)(sel_id + 1);
            plot_growth[idx] = 0;
            plot_water[idx] = 0;
            seeds[sel_id]--;
            energy--;
            advance_minutes(2);
            sound_play(SFX_PLANT);
            set_message("planted");
            return 1;
        }

        int cid = plot_crop[idx] - 1;
        if (plot_growth[idx] >= crops[cid].days) {
            produce[cid]++;
            if (crops[cid].regrow) {
                plot_growth[idx] = (uint8_t)(crops[cid].days - crops[cid].regrow);
            } else {
                plot_crop[idx] = 0;
                plot_growth[idx] = 0;
            }
            plot_water[idx] = 0;
            advance_minutes(2);
            sound_play(SFX_HARVEST);
            set_message_plus(crops[cid].name);
            return 1;
        }

        if (!plot_water[idx]) {
            if (sel_kind != IT_CAN) { sound_play(SFX_ERROR); set_message("select can"); return 1; }
            if (energy <= 0) { sound_play(SFX_ERROR); set_message("too tired"); return 1; }
            if (water_can <= 0) { sound_play(SFX_ERROR); set_message("can empty"); return 1; }
            plot_water[idx] = 1;
            water_can--;
            energy--;
            advance_minutes(2);
            sound_play(SFX_WATER);
            set_message("watered");
            return 1;
        }
        set_message("wet");
        return 1;
    }

    set_message("not here");
    return 1;
}

static void clear_plots(void)
{
    for (int i = 0; i < PLOTS; i++) {
        plot_crop[i] = 0;
        plot_growth[i] = 0;
        plot_water[i] = 0;
    }
}

static void sleep_night(void)
{
    sound_play(SFX_SLEEP);
    int old_season = season;
    int old_day = day;
    int rain = (weather == 1);

    summary_gold = 0;
    for (int i = 0; i < 16; i++) {
        summary_items[i] = bin_items[i];
        if (bin_items[i]) summary_gold += bin_items[i] * crops[i].sell;
        bin_items[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        summary_fish[i] = bin_fish[i];
        if (bin_fish[i]) summary_gold += bin_fish[i] * fish_price[i];
        bin_fish[i] = 0;
    }
    summary_open = summary_gold > 0;
    coins += bin_value;
    bin_value = 0;

    for (int i = 0; i < PLOTS; i++) {
        if (plot_crop[i] && (plot_water[i] || rain)) {
            if (plot_growth[i] < 15) plot_growth[i]++;
        }
        plot_water[i] = 0;
    }

    day++;
    if (day > 28) {
        day = 1;
        season = (season + 1) & 3;
        if (season == 0) year++;
        clear_plots();
        sel_kind = IT_SEED;
        sel_id = season * 4;
        palette_set(season);
        tiles_build();
        set_message("new season");
    } else {
        if (day == 28) set_message("last day");
        else if (day == 27) set_message("2 days left");
        else if (day == 26) set_message("3 days left");
        else set_message("good morning");
    }

    shell_mask = 0;
    pet_fed = 0;
    wish_done = 0;
    dolphin_done = 0;
    cat_bed_block = 0;

    if ((pet_owned & 1) && pet_aff[0] >= 100) {
        int t = rnd() & 3;
        if (fish[t] < 15) fish[t]++;
    }
    if ((pet_owned & 2) && pet_aff[1] >= 100) {
        if (produce[15] < 15) produce[15]++;
    }
    if ((pet_owned & 3) == 3 && pet_aff[0] >= 100 && pet_aff[1] >= 100 && !dino_unlocked) {
        dino_unlocked = 1;
        dino_x = 13 * TILE;
        dino_y = 12 * TILE;
    }
    if (dino_unlocked) {
        if (ore_count < 255) ore_count++;
    }

    if ((pet_owned & 3) == 3 && pet_aff[0] >= 100 && pet_aff[1] >= 100 && dino_unlocked)
        set_message("dino friend!");
    else if (dino_unlocked)
        set_message("dino ore!");
    else if (((pet_owned & 1) && pet_aff[0] >= 100) || ((pet_owned & 2) && pet_aff[1] >= 100))
        set_message("pet gift!");

    energy = energy_max;
    minutes = 6 * 60;
    time_ticks = 0;
    weather = weather_next();
    if (wish_rain) { weather = 1; wish_rain = 0; }

    sleep_timer = 45;
    {
        int p = 0;
        if (season != old_season) {
            const char *sn = season_name[season];
            while (*sn && p < 10) sleep_text[p++] = *sn++;
            sleep_text[p++] = ' ';
        } else {
            sleep_text[p++] = 'd';
            sleep_text[p++] = 'a';
            sleep_text[p++] = 'y';
            sleep_text[p++] = ' ';
        }
        if (day >= 10) {
            sleep_text[p++] = (char)('0' + day / 10);
            sleep_text[p++] = (char)('0' + day % 10);
        } else {
            sleep_text[p++] = (char)('0' + day);
        }
        sleep_text[p] = 0;
    }

    for (int i = 0; i < 3; i++) {
        npc_talked[i] = 0;
        npc_gifted[i] = 0;
    }

    SaveData sd;
    game_to_save(&sd);
    save_write(&sd);

    int all = (npc_points[0] >= 60 && npc_points[1] >= 60 && npc_points[2] >= 60);

    /* No money finish line. The year-one goal is to bring the town back:
       finish all three storylines and become 3-heart friends with everyone. */
    if (old_season == 3 && old_day == 28 && all_stories_done() && all) {
        int best = (npc_points[0] >= 100 && npc_points[1] >= 100 && npc_points[2] >= 100);
        ending = (best && dino_unlocked) ? 3 : 2;
        state = ST_WIN;
    }
}


static int quest_have(int i)
{
    int qi = (story[i] == 1) ? 0 : 1;
    const Quest *q = &quests[i][qi];
    if (q->kind == 0) return produce[q->id] >= q->count;
    if (q->kind == 1) return fish[q->id] >= q->count;
    return shell_count >= q->count;
}

static void quest_take(int i)
{
    int qi = (story[i] == 1) ? 0 : 1;
    const Quest *q = &quests[i][qi];
    if (q->kind == 0) produce[q->id] -= q->count;
    else if (q->kind == 1) fish[q->id] -= q->count;
    else shell_count -= q->count;
}

static char story_marker(int i)
{
    if (story[i] == 0 && npc_points[i] >= 40) return '!';
    if (story[i] == 2 && npc_points[i] >= 80) return '!';
    if (story[i] == 1 || story[i] == 3) return quest_have(i) ? '?' : '!';
    return 0;
}

static void story_desc(int i, char *out)
{
    int st = story[i];
    if (st == 0) {
        const char *s = "needs 2 hearts";
        while (*s) *out++ = *s++;
        *out = 0;
        return;
    }
    if (st == 2) {
        const char *s = "chapter 1 done";
        while (*s) *out++ = *s++;
        *out = 0;
        return;
    }
    if (st == 4) {
        const char *s = "story done";
        while (*s) *out++ = *s++;
        *out = 0;
        return;
    }
    int qi = (st == 1) ? 0 : 1;
    const Quest *q = &quests[i][qi];
    const char *nm = (q->kind == 0) ? crops[q->id].name :
                     (q->kind == 1) ? fish_name[q->id] : "shell";
    while (*nm) *out++ = *nm++;
    *out++ = ' ';
    int have = 0;
    if (q->kind == 0) have = produce[q->id];
    else if (q->kind == 1) have = fish[q->id];
    else have = shell_count;
    if (have > 99) have = 99;
    if (have >= 10) *out++ = (char)('0' + have / 10);
    *out++ = (char)('0' + have % 10);
    *out++ = '/';
    *out++ = (char)('0' + q->count);
    *out = 0;
}

static int story_talk(int i)
{
    int st = story[i];
    if ((st == 0 && npc_points[i] >= 40) || (st == 2 && npc_points[i] >= 80)) {
        int chapter = (st == 2) ? 1 : 0;
        story[i] = (uint8_t)(chapter ? 3 : 1);
        for (int k = 0; k < 4; k++) dlg_lines[k] = story_lines[i][chapter].start[k];
        dlg_npc = i;
        dlg_page = 0;
        dlg_pages = 4;
        dlg_open = 1;
        save_now();
        sound_play(SFX_GIFT);
        return 1;
    }
    if (st == 1 || st == 3) {
        int qi = (st == 1) ? 0 : 1;
        const Quest *q = &quests[i][qi];
        if (quest_have(i)) {
            quest_take(i);
            coins += q->gold;
            seeds[q->seed_id] += q->seed_count;
            npc_points[i] += 20;
            if (npc_points[i] > 100) npc_points[i] = 100;
            story[i]++;
            save_now();
            for (int k = 0; k < 3; k++) dlg_lines[k] = story_lines[i][qi].done[k];
            dlg_npc = i;
            dlg_page = 0;
            dlg_pages = 3;
            dlg_open = 1;
            if (mine_unlocked()) set_message("mine open");
            sound_play(SFX_UPGRADE);
            return 1;
        }
        dlg_lines[0] = story_lines[i][qi].ask[0];
        dlg_lines[1] = story_lines[i][qi].ask[1];
        dlg_lines[2] = "not enough yet.";
        dlg_npc = i;
        dlg_page = 0;
        dlg_pages = 3;
        dlg_open = 1;
        sound_play(SFX_CLICK);
        return 1;
    }
    return 0;
}

static const char *special_line(int i)
{
    if (i == 1 && weather == 1) return "dad loved rain.";
    if (i == 0 && minutes < 7 * 60) return "i woke up early.";
    if (i == 2 && minutes >= 17 * 60) return "the sea is pink.";
    if (i == 1 && weather == 2) return "dad's boots are cold.";
    return 0;
}

static void npc_talk(int i)
{
    if (story_talk(i)) return;
    if (i == 0 && (secrets & SEC_GOLDEN) && !(secrets & SEC_GOLD_TOLD)) {
        dlg_lines[0] = "a golden fish!";
        dlg_lines[1] = "where did you...";
        dlg_npc = i;
        dlg_page = 0;
        dlg_pages = 2;
        dlg_open = 1;
        secrets |= SEC_GOLD_TOLD;
        save_now();
        return;
    }
    advance_minutes(1);
    int pages = 0;
    int first = (npc_points[i] == 0 && !npc_talked[i]);

    if (first) {
        dlg_lines[pages++] = npc_def[i].first[0];
        dlg_lines[pages++] = npc_def[i].first[1];
        dlg_lines[pages++] = npc_def[i].season[season];
    } else {
        int tier = npc_points[i] < 40 ? 0 : (npc_points[i] < 80 ? 1 : 2);
        dlg_lines[pages++] = npc_def[i].lines[tier][0];
        dlg_lines[pages++] = npc_def[i].lines[tier][1];
        const char *sp = special_line(i);
        dlg_lines[pages++] = sp ? sp : npc_def[i].season[season];
        if (weather == 1) dlg_lines[pages++] = npc_def[i].rain[0];
        else if (weather == 2) dlg_lines[pages++] = npc_def[i].snow[0];
    }

    dlg_npc = i;
    dlg_page = 0;
    dlg_pages = pages;
    dlg_open = 1;
    if (!npc_talked[i]) {
        npc_points[i]++;
        npc_talked[i] = 1;
    }
}

static void npc_gift(int i, int kind, int id)
{
    advance_minutes(1);
    int fav = (kind == IT_CROP && id + 1 == npc_def[i].favorite);
    int p = 0;
    const char *name = item_name(kind, id);
    while (*name && p < 20) gift_buf[p++] = *name++;
    gift_buf[p++] = '!';
    gift_buf[p] = 0;

    npc_points[i] += fav ? 5 : 2;
    if (npc_points[i] > 100) npc_points[i] = 100;
    npc_gifted[i] = 1;
    if (kind == IT_CROP) produce[id]--;
    else if (kind == IT_FISH) fish[id]--;

    dlg_lines[0] = gift_buf;
    dlg_lines[1] = (i == 0 && kind == IT_FISH) ? "grandma's soup!" :
                  (fav ? "i love it!" : "thank you!");
    dlg_npc = i;
    dlg_page = 0;
    dlg_pages = 2;
    dlg_open = 1;
    sound_play(SFX_GIFT);
}

static void save_now(void)
{
    SaveData d;
    game_to_save(&d);
    save_write(&d);
}

static void upgrade_house(void)
{
    if (house_level >= 4) {
        status_open = 0;
        set_message("house max");
        return;
    }
    int cost = house_cost[house_level];
    if (coins < cost) {
        status_open = 0;
        set_message("not enough");
        return;
    }
    coins -= cost;
    house_level++;
    energy_max = 30 + house_level * 5;
    build_world();
    palette_set(season);
    tiles_build();
    save_now();
    status_open = 0;
    sound_play(SFX_UPGRADE);
    set_message("house up");
}

static void upgrade_can(void)
{
    if (can_level >= 2) {
        status_open = 0;
        set_message("can max");
        return;
    }
    int cost = can_cost[can_level];
    if (coins < cost) {
        status_open = 0;
        set_message("not enough");
        return;
    }
    coins -= cost;
    can_level++;
    water_can_max = can_cap[can_level];
    water_can = water_can_max;
    save_now();
    status_open = 0;
    sound_play(SFX_UPGRADE);
    set_message("can up");
}

/* --- save -------------------------------------------------------------- */

static void game_to_save(SaveData *d)
{
    memset(d, 0, sizeof(*d));
    d->year = (int16_t)year;
    d->season = (int16_t)season;
    d->day = (int16_t)day;
    d->minutes = (int16_t)minutes;
    d->coins = (int16_t)coins;
    d->energy = (int16_t)energy;
    d->bin_value = (int16_t)bin_value;
    d->water_can = (uint8_t)water_can;
    d->scene = (int16_t)scene_id;
    d->px = (int16_t)player_x;
    d->py = (int16_t)player_y;
    d->dir = (int16_t)facing;
    d->selected = (uint8_t)((sel_kind << 5) | (sel_id & 31));
    d->weather = (uint8_t)weather;
    d->flags = (uint8_t)(((can_level & 3) << 2) | (rod_owned ? 16 : 0));
    d->pad0 = (uint8_t)((shell_mask & 15) | ((house_level & 15) << 4));
    d->pet_owned = (uint8_t)pet_owned;
    d->pet_aff[0] = pet_aff[0];
    d->pet_aff[1] = pet_aff[1];
    d->pet_fed = pet_fed;
    d->dino_unlocked = (uint8_t)dino_unlocked;
    d->pad[1] = (uint8_t)((fish[0] & 15) | ((fish[1] & 15) << 4));
    d->pad[2] = (uint8_t)((fish[2] & 15) | ((fish[3] & 15) << 4));
    memcpy(d->story, story, sizeof(story));
    d->shell_count = shell_count;
    d->ore_count = ore_count;
    d->player_hp = (uint8_t)player_hp;
    d->mine_floor = 1;
    d->mine_pad = secrets;
    memcpy(d->seeds, seeds, 16);
    memcpy(d->produce, produce, 16);
    memcpy(d->bin_items, bin_items, 16);
    memcpy(d->crop_id, plot_crop, PLOTS);
    memcpy(d->crop_growth, plot_growth, PLOTS);
    memcpy(d->crop_water, plot_water, PLOTS);
    for (int i = 0; i < 3; i++) {
        d->npc_points[i] = (uint8_t)npc_points[i];
        d->npc_talked[i] = (uint8_t)npc_talked[i];
        d->npc_gifted[i] = (uint8_t)npc_gifted[i];
    }
}

static void game_from_save(const SaveData *d)
{
    year = d->year;
    season = d->season;
    day = d->day;
    minutes = d->minutes;
    coins = d->coins;
    energy = d->energy;
    bin_value = d->bin_value;
    water_can = d->water_can;
    sel_kind = (d->selected >> 5) & 7;
    sel_id = d->selected & 31;
    if (sel_kind < IT_HOE || sel_kind > IT_FISH) { sel_kind = IT_SEED; sel_id = season * 4; }
    if (sel_kind == IT_ROD && !rod_owned) { sel_kind = IT_CAN; sel_id = 0; }
    if (sel_kind == IT_SWORD && !(d->story[0] >= 4 && d->story[1] >= 4 && d->story[2] >= 4)) {
        sel_kind = IT_CAN; sel_id = 0;
    }
    weather = d->weather;
    house_level = (d->pad0 >> 4) & 15;
    if (house_level == 0 && (d->flags & 3) != 0) house_level = d->flags & 3;
    if (house_level > 4) house_level = 4;
    can_level = (d->flags >> 2) & 3;
    if (can_level > 2) can_level = 2;
    rod_owned = (d->flags >> 4) & 1;
    shell_mask = d->pad0 & 15;
    pet_owned = d->pet_owned & 3;
    pet_aff[0] = d->pet_aff[0] > 100 ? 100 : d->pet_aff[0];
    pet_aff[1] = d->pet_aff[1] > 100 ? 100 : d->pet_aff[1];
    pet_fed = d->pet_fed & 3;
    dino_unlocked = d->dino_unlocked & 1;
    if (!pet_owned && (d->pad[0] & 3)) {
        int oldt = d->pad[0] & 3;
        if (oldt == 1) { pet_owned |= 1; pet_aff[0] = (d->pad[0] >> 3) * 20; }
        else if (oldt == 2) { pet_owned |= 2; pet_aff[1] = (d->pad[0] >> 3) * 20; }
        if ((d->pad[0] >> 2) & 1) pet_fed |= (oldt == 1) ? 1 : 2;
        dino_unlocked = 0;
    }
    memcpy(story, d->story, sizeof(story));
    shell_count = d->shell_count;
    ore_count = d->ore_count;
    secrets = d->mine_pad;
    player_hp = d->player_hp ? d->player_hp : 3;
    if (player_hp > 3) player_hp = 3;
    for (int i = 0; i < 3; i++) if (story[i] > 4) story[i] = 4;
    for (int i = 0; i < 2; i++) {
        pet_x[i] = (10 + i * 3) * TILE;
        pet_y[i] = 10 * TILE;
        pet_dir[i] = 0;
        pet_frame[i] = 0;
        pet_wait[i] = 0;
    }
    dino_x = 13 * TILE;
    dino_y = 12 * TILE;
    dino_frame = 0;
    dino_wait = 0;
    fish[0] = d->pad[1] & 15;
    fish[1] = (d->pad[1] >> 4) & 15;
    fish[2] = d->pad[2] & 15;
    fish[3] = (d->pad[2] >> 4) & 15;
    energy_max = 30 + house_level * 5;
    water_can_max = can_cap[can_level];
    memcpy(seeds, d->seeds, 16);
    memcpy(produce, d->produce, 16);
    memcpy(bin_items, d->bin_items, 16);
    memcpy(plot_crop, d->crop_id, PLOTS);
    memcpy(plot_growth, d->crop_growth, PLOTS);
    memcpy(plot_water, d->crop_water, PLOTS);
    for (int i = 0; i < 3; i++) {
        npc_points[i] = d->npc_points[i];
        npc_talked[i] = d->npc_talked[i];
        npc_gifted[i] = d->npc_gifted[i];
    }
    scene_set(d->scene);
    if (scene_id < 0 || scene_id >= SC_COUNT) scene_set(SC_FARM);
    player_x = clamp(d->px, 0, scene_w[scene_id] * TILE - TILE);
    player_y = clamp(d->py, 0, scene_h[scene_id] * TILE - TILE);
    facing = d->dir;
    minutes = clamp(minutes, 360, 1439);
    if (energy < 0 || energy > energy_max) energy = energy_max;
    if (season < 0 || season > 3) season = 0;
    if (day < 1 || day > 28) day = 1;
    if (coins < 0) coins = 0;
    if (bin_value < 0) bin_value = 0;
    if (water_can < 0 || water_can > water_can_max) water_can = water_can_max;
    if (player_x < 0) player_x = 0;
    if (player_y < 0) player_y = 0;
    build_world();
    palette_set(season);
    tiles_build();
}

static void game_new(void)
{
    season = 0;
    year = 1;
    day = 1;
    minutes = 6 * 60;
    coins = 50;
    house_level = 0;
    can_level = 0;
    rod_owned = 0;
    shell_mask = 0;
    memset(fish, 0, sizeof(fish));
    memset(bin_fish, 0, sizeof(bin_fish));
    pet_owned = 0;
    pet_aff[0] = pet_aff[1] = 0;
    pet_fed = 0;
    dino_unlocked = 0;
    for (int i = 0; i < 2; i++) {
        pet_x[i] = (10 + i * 3) * TILE;
        pet_y[i] = 10 * TILE;
        pet_dir[i] = 0;
        pet_frame[i] = 0;
        pet_wait[i] = 0;
    }
    dino_x = 13 * TILE;
    dino_y = 12 * TILE;
    dino_frame = 0;
    dino_wait = 0;
    memset(story, 0, sizeof(story));
    shell_count = 0;
    ore_count = 0;
    player_hp = 3;
    secrets = 0;
    wish_done = wish_rain = dolphin_done = dolphin_timer = cat_bed_block = 0;
    energy_max = 30;
    water_can_max = 15;
    energy = energy_max;
    bin_value = 0;
    water_can = water_can_max;
    sel_kind = IT_SEED;
    sel_id = 0;
    weather = 0;
    memset(seeds, 0, sizeof(seeds));
    memset(produce, 0, sizeof(produce));
    memset(bin_items, 0, sizeof(bin_items));
    seeds[0] = 5;
    seeds[1] = 3;
    seeds[2] = 2;
    seeds[3] = 2;
    clear_plots();

    int carrot = plot_index(12, 10);
    int potato = plot_index(14, 10);
    if (carrot >= 0) { plot_crop[carrot] = 1; plot_growth[carrot] = 1; }
    if (potato >= 0) { plot_crop[potato] = 2; plot_growth[potato] = 2; }

    player_x = 9 * TILE;
    player_y = 9 * TILE;
    facing = 1;
    player_walk = 0;
    player_walk_tick = 0;

    for (int i = 0; i < 3; i++) {
        npc_points[i] = 0;
        npc_talked[i] = 0;
        npc_gifted[i] = 0;
        npcs[i].scene = -1;
        npcs[i].x = 0;
        npcs[i].y = 0;
        npcs[i].dir = 0;
        npcs[i].frame = 0;
        npcs[i].moving = 0;
        npcs[i].wx = 0;
        npcs[i].wy = 0;
        npcs[i].wait = 0;
        npcs[i].wander = 0;
        npcs[i].step = 0;
    }

    scene_set(SC_FARM);
    build_world();
    palette_set(season);
    tiles_build();
    state = ST_TITLE;
    title_t = 0;
    message = 0;
    message_timer = 0;
    shop_open = 0;
    bin_open = 0;
    summary_open = 0;
    dlg_open = 0;
    status_open = 0;
    fade_timer = 0;
    sleep_timer = 0;
    time_ticks = 0;
    ending = 0;
}

void game_debug_warp(int scene, int tx, int ty)
{
    scene_set(scene);
    player_x = tx * TILE;
    player_y = ty * TILE;
}

void game_debug_time(int mins)
{
    minutes = mins;
    for (int i = 0; i < 3; i++) {
        npc_update(i);
    }
}

void game_debug_weather(int w)
{
    weather = w;
}

void game_debug_season(int s)
{
    season = s & 3;
    if (sel_kind == IT_SEED && sel_id / 4 != season) sel_id = season * 4;
    clear_plots();
    palette_set(season);
    tiles_build();
}

void game_debug_give(int crop_id, int n)
{
    if (crop_id >= 1 && crop_id <= 16) produce[crop_id - 1] += n;
}

void game_debug_fill_pets(void)
{
    pet_owned = 3;
    pet_aff[0] = pet_aff[1] = 100;
    pet_fed = 0;
    dino_unlocked = 1;
}

void game_debug_unlock_mine(void)
{
    story[0] = story[1] = story[2] = 4;
}

void game_debug_tool(int kind)
{
    if (kind >= IT_HOE && kind <= IT_SWORD) { sel_kind = kind; sel_id = 0; }
}

void game_debug_select(int s)
{
    int count = season_crop_count[season];
    if (s < count) { sel_kind = IT_SEED; sel_id = season * 4 + s; }
    else { sel_kind = IT_CROP; sel_id = season * 4 + (s - count); }
}
void game_debug_coins(int c) { coins = c; }
void game_debug_points(int i, int v) { if (i >= 0 && i < 3) npc_points[i] = v; }
void game_debug_day(int d) { day = d; }

void game_debug_set_heart(int i, int p) { if (i >= 0 && i < 3) npc_points[i] = p; }
void game_debug_give_crop(int id, int n) { if (id >= 0 && id < 16) produce[id] = (uint8_t)(produce[id] + n); }
int game_debug_points_get(int i){return (i>=0&&i<3)?npc_points[i]:0;}
int game_debug_story_get(int i){return (i>=0&&i<3)?story[i]:0;}
int game_debug_coins_now(void) { return coins; }
int game_debug_house_now(void) { return house_level; }

void game_debug_give_fish(int type, int n)
{
    if (type >= 0 && type < 4) fish[type] = (uint8_t)(fish[type] + n);
}

int game_debug_target_tile(void)
{
    int tx, ty;
    target_tile(&tx, &ty);
    return map_get(tx, ty);
}

void game_debug_fish_bite(void)
{
    if (fish_state == FISH_WAIT) {
        fish_state = FISH_BITE;
        fish_timer = 600;
    }
}

void game_debug_play(void)
{
    state = ST_PLAY;
    ending = 0;
    sleep_timer = 0;
    fade_timer = 0;
    status_open = 0;
    dlg_open = 0;
    shop_open = 0;
    message = 0;
    message_timer = 0;
}

void game_init(void)
{
    build_world();
    palette_set(0);
    tiles_build();

    SaveData d;
    boot_loaded = save_read(&d);
    if (boot_loaded) {
        game_from_save(&d);
        set_message("welcome back");
    } else {
        game_new();
    }
    state = ST_TITLE;
    title_t = 0;

    a_prev = b_prev = menu_prev = up_prev = down_prev = 0;
    time_ticks = 0;
    ending = 0;
}

int game_update(const Input *in)
{
    int dirty = 0;

    if (message_timer > 0) {
        message_timer--;
        dirty = 1;
    }

    int a_edge = in->a && !a_prev;
    int b_edge = in->b && !b_prev;
    int m_edge = in->menu && !menu_prev;
    int l_edge = in->left && !left_prev;
    int r_edge = in->right && !right_prev;
    a_prev = in->a;
    b_prev = in->b;
    menu_prev = in->menu;
    left_prev = in->left;
    right_prev = in->right;
    if (in->a) {
        if (a_hold < 1000) a_hold++;
    } else {
        a_hold = 0;
    }

    if (fade_timer > 0) {
        fade_timer--;
        if (fade_timer == 10) {
            scene_set(fade_scene);
            player_x = fade_x;
            player_y = fade_y;
            facing = fade_dir;
            shop_open = 0;
            dlg_open = 0;
        }
        return 1;
    }

    if (state == ST_TITLE) {
        if (a_edge) {
            state = ST_PLAY;
            set_message(boot_loaded ? "welcome back" : "help the town");
        } else if (title_t < TITLE_END) {
            title_t++;
        }
        return 1;
    }

    if (state == ST_WIN) {
        if (a_edge) game_new();
        return 1;
    }

    if (sleep_timer > 0) {
        sleep_timer--;
        return 1;
    }

    if (summary_open) {
        if (a_edge || b_edge) summary_open = 0;
        return 1;
    }

    if (bin_open) {
        if (a_edge) deposit_bin();
        if (b_edge) bin_open = 0;
        return 1;
    }

    if (pet_shop_open) {
        int u_edge = in->up && !up_prev;
        int d_edge = in->down && !down_prev;
        up_prev = in->up;
        down_prev = in->down;
        if (u_edge) pet_shop_sel = (pet_shop_sel + 2) % 3;
        if (d_edge) pet_shop_sel = (pet_shop_sel + 1) % 3;
        if (b_edge) pet_shop_open = 0;
        if (a_edge) {
            if (pet_shop_sel == 2) {
                pet_shop_open = 0;
            } else {
                int pi = pet_shop_sel;
                int t = pi + 1;
                if (pet_owned & (1 << pi)) {
                    pet_shop_open = 0;
                    set_message("have pet");
                } else if (coins >= pet_price[t]) {
                    coins -= pet_price[t];
                    pet_owned |= (1 << pi);
                    pet_fed &= (uint8_t)~(1 << pi);
                    pet_aff[pi] = 0;
                    save_now();
                    pet_shop_open = 0;
                    sound_play(SFX_PET);
                    set_message(t == 1 ? "cat!" : "chicken!");
                } else {
                    pet_shop_open = 0;
                    set_message("not enough");
                }
            }
        }
        return 1;
    }

    if (menu_open) {
        if (menu_page == 2) {
            if (a_edge) {
                SaveData md;
                if (save_read(&md)) {
                    game_from_save(&md);
                    state = ST_PLAY;
                    menu_open = 0;
                    set_message("loaded");
                    for (int i = 0; i < 3; i++) npc_update(i);
                } else {
                    menu_open = 0;
                    menu_page = 0;
                    set_message("no save");
                }
            }
            if (b_edge) menu_page = 0;
            return 1;
        }
        if (menu_page == 1) {
            int u_edge = in->up && !up_prev;
            int d_edge = in->down && !down_prev;
            up_prev = in->up;
            down_prev = in->down;
            if (l_edge && (bag_sel % 6) != 0) bag_sel--;
            if (r_edge && (bag_sel % 6) != 5) bag_sel++;
            if (u_edge && bag_sel >= 6) bag_sel -= 6;
            if (d_edge && bag_sel < 24) bag_sel += 6;
            if (a_edge) {
                int kind, id, count;
                if (bag_get(bag_sel, &kind, &id, &count)) {
                    sel_kind = kind;
                    sel_id = id;
                    menu_open = 0;
                }
            }
            if (b_edge || m_edge) menu_open = 0;
            return 1;
        }

        int u_edge = in->up && !up_prev;
        int d_edge = in->down && !down_prev;
        up_prev = in->up;
        down_prev = in->down;
        if (u_edge) menu_sel = (menu_sel + 6) % 7;
        if (d_edge) menu_sel = (menu_sel + 1) % 7;

        if (m_edge || b_edge) { menu_open = 0; return 1; }
        if (a_edge) {
            if (menu_sel == 0) {
                menu_open = 0;
            } else if (menu_sel == 1) {
                menu_page = 1;
                bag_sel = bag_find_selected();
            } else if (menu_sel == 2) {
                menu_open = 0;
                status_open = 1;
                status_page = 0;
            } else if (menu_sel == 3) {
                if (scene_id == SC_HOME) {
                    menu_open = 0;
                    status_open = 1;
                    status_page = 3;
                    status_upg_sel = 0;
                } else {
                    menu_open = 0;
                    set_message("go home");
                }
            } else if (menu_sel == 4) {
                SaveData md;
                game_to_save(&md);
                save_write(&md);
                menu_open = 0;
                set_message("saved");
            } else if (menu_sel == 5) {
                menu_page = 2;
            } else if (menu_sel == 6) {
                menu_open = 0;
                if (player_near_bed()) {
                    sleep_night();
                } else {
                    set_message("go to bed");
                }
            }
        }
        return 1;
    }

    if (dlg_open) {
        if (a_edge) {
            dlg_page++;
            if (dlg_page >= dlg_pages) dlg_open = 0;
        }
        if (b_edge) dlg_open = 0;
        return 1;
    }

    if (status_open) {
        if (status_page == 3) {
            int u_edge = in->up && !up_prev;
            int d_edge = in->down && !down_prev;
            up_prev = in->up;
            down_prev = in->down;
            if (u_edge || d_edge) status_upg_sel ^= 1;
            if (a_edge) {
                if (status_upg_sel == 0) upgrade_house();
                else upgrade_can();
            }
            if (b_edge) status_open = 0;
            return 1;
        }
        if (a_edge && status_page < 3) status_page++;
        if (b_edge) status_open = 0;
        return 1;
    }

    if (shop_open) {
        int u_edge = in->up && !up_prev;
        int d_edge = in->down && !down_prev;
        up_prev = in->up;
        down_prev = in->down;
        int count = season_crop_count[season];
        if (u_edge) { shop_sel = (shop_sel + count - 1) % count; dirty = 1; }
        if (d_edge) { shop_sel = (shop_sel + 1) % count; dirty = 1; }
        if (l_edge || r_edge) {
            shop_qty = shop_qty == 1 ? 5 : (shop_qty == 5 ? 10 : 1);
            dirty = 1;
        }
        if (a_edge) {
            int id = season * 4 + shop_sel;
            int total = crops[id].price * shop_qty;
            if (coins >= total) {
                coins -= total;
                seeds[id] += (uint8_t)shop_qty;
                sel_kind = IT_SEED;
                sel_id = id;
                sound_play(SFX_COIN);
                set_message("bought");
            } else {
                set_message("not enough");
            }
            dirty = 1;
        }
        if (b_edge) { shop_open = 0; dirty = 1; }
        return dirty | 1;
    }
    up_prev = in->up;
    down_prev = in->down;

    if (fish_state != FISH_IDLE) {
        fish_update(in, a_edge, b_edge);
        return 1;
    }

    if (m_edge) {
        menu_open = 1;
        menu_page = 0;
        menu_sel = 0;
        sound_play(SFX_CLICK);
        return 1;
    }

    if (++time_ticks >= 50) {
        time_ticks = 0;
        minutes++;
        if (minutes >= 24 * 60) sleep_night();
        dirty = 1;
    }

    for (int i = 0; i < 3; i++) {
        if (npc_update(i)) dirty = 1;
    }
    pet_update();
    mine_update();

    int old_x = player_x;
    int old_y = player_y;
    int dx = 0, dy = 0;
    if (in->left) { dx -= 2; facing = 2; }
    if (in->right) { dx += 2; facing = 3; }
    if (in->up) { dy -= 2; facing = 1; }
    if (in->down) { dy += 2; facing = 0; }
    if (dx || dy) {
        int fx = (player_x + 8) / TILE;
        int fy = (player_y + 12) / TILE;
        int tx = fx, ty = fy;
        if (in->up) ty--;
        else if (in->down) ty++;
        else if (in->left) tx--;
        else if (in->right) tx++;
        const Door *bd = door_at(scene_id, tx, ty);
        if (bd) {
            start_fade(bd);
            return 1;
        }
    }
    if (dx && can_stand(player_x + dx, player_y)) player_x += dx;
    if (dy && can_stand(player_x, player_y + dy)) player_y += dy;

    if (player_x != old_x || player_y != old_y) {
        if (++player_walk_tick >= 5) {
            player_walk = (player_walk + 1) & 3;
            player_walk_tick = 0;
        }
    } else {
        player_walk = 0;
        player_walk_tick = 0;
    }

    {
        int n = npc_near();
        int sh = shell_near();
        int tx, ty;
        target_tile(&tx, &ty);
        int at_fountain = (scene_id == SC_TOWN && map_get(tx, ty) == T_FOUNTAIN);
        int act = a_edge;
        if (!act && in->a && (a_hold % 20) == 0 &&
            (n >= 0 || door_adjacent() || sh >= 0 || at_fountain)) {
            act = 1;
        }
        if (act) {
            if (at_fountain) {
                use_tool();
            } else if (sh >= 0) {
                shell_mask |= (uint8_t)(1 << sh);
                if (shell_count < 255) shell_count++;
                coins += 15;
                advance_minutes(2);
                sound_play(SFX_COIN);
                set_message("+15g shell");
            } else if (n >= 0) {
                if (act) npc_talk(n);
            } else {
                use_tool();
            }
            a_hold = 0;
            dirty = 1;
        }
    }

    if (b_edge) {
        int n = npc_near();
        if (n >= 0 && !npc_gifted[n]) {
            if ((sel_kind == IT_CROP || sel_kind == IT_FISH) && sel_count() > 0) {
                npc_gift(n, sel_kind, sel_id);
                dirty = 1;
            } else {
                set_message("select gift");
            }
        }
    }

    if (dolphin_timer > 0) { dolphin_timer--; dirty = 1; }
    water_anim++;
    if ((water_anim & 15) == 0) dirty = 1;
    if (player_x != old_x || player_y != old_y) dirty = 1;
    return dirty;
}

/* --- collision --------------------------------------------------------- */

static int solid(uint8_t t)
{
    return t == T_WATER || t == T_TREE || t == T_WALL || t == T_WINDOW ||
           t == T_ROOF || t == T_DOOR || t == T_FENCE || t == T_CHIMNEY ||
           t == T_STAND || t == T_BIN || t == T_LAMP || t == T_FOUNTAIN ||
           t == T_WALL_IN || t == T_BED || t == T_TABLE || t == T_COUNTER ||
           t == T_SHELF || t == T_PLANT || t == T_STOVE || t == T_DOOR_IN ||
           t == T_WINDOW_IN || t == T_CHAIR || t == T_BOOKSHELF || t == T_CRATE ||
           t == T_AWNING || t == T_SIGN || t == T_SCARECROW ||
           t == T_BARREL || t == T_MAILBOX || t == T_BENCH || t == T_ROCK ||
           t == T_PAINTING || t == T_CLOCK || t == T_ROOF2 || t == T_WALL2 ||
           t == T_WALL_IN2 || t == T_BOWL || t == T_ROOF3 ||
           t == T_MINE_WALL || t == T_ORE || t == T_BOWL2;
}

static int can_stand(int px, int py)
{
    int x0 = (px + 2) / TILE;
    int x1 = (px + 13) / TILE;
    int y0 = (py + 7) / TILE;
    int y1 = (py + 15) / TILE;

    return !solid(map_get(x0, y0)) && !solid(map_get(x1, y0)) &&
           !solid(map_get(x0, y1)) && !solid(map_get(x1, y1));
}

/* --- render ------------------------------------------------------------ */

static void draw_bed(uint16_t *fb, int x, int y)
{
    fill_rect(fb, x + 2, y + 28, 28, 3, RGB(52, 34, 24));
    fill_rect(fb, x + 1, y + 2, 30, 28, C_WOOD);
    fill_rect(fb, x + 2, y + 3, 28, 26, RGB(196, 150, 92));
    fill_rect(fb, x, y, 3, 31, C_WOOD);
    fill_rect(fb, x + 29, y, 3, 31, C_WOOD);
    fill_rect(fb, x, y, 32, 6, RGB(214, 168, 104));
    fill_rect(fb, x + 2, y + 1, 28, 1, RGB(246, 204, 138));
    fill_rect(fb, x + 2, y + 6, 28, 18, RGB(248, 244, 230));
    fill_rect(fb, x + 4, y + 8, 10, 8, C_WHITE);
    fill_rect(fb, x + 18, y + 8, 10, 8, C_WHITE);
    fill_rect(fb, x + 4, y + 14, 10, 1, RGB(216, 210, 196));
    fill_rect(fb, x + 18, y + 14, 10, 1, RGB(216, 210, 196));
    fill_rect(fb, x + 2, y + 17, 28, 11, RGB(186, 66, 68));
    fill_rect(fb, x + 2, y + 17, 28, 2, RGB(224, 96, 92));
    fill_rect(fb, x + 7, y + 18, 2, 9, RGB(150, 50, 54));
    fill_rect(fb, x + 15, y + 18, 2, 9, RGB(150, 50, 54));
    fill_rect(fb, x + 23, y + 18, 2, 9, RGB(150, 50, 54));
    fill_rect(fb, x + 2, y + 28, 28, 2, RGB(110, 74, 42));
    fill_rect(fb, x + 1, y + 16, 30, 1, RGB(104, 72, 42));
}

static void draw_tile(uint16_t *fb, int dx, int dy, uint8_t t)
{
    const uint16_t *src = tile_img[t];
    if (view_num == view_den) {
        for (int x = 0; x < TILE; x++) {
            int sx = dx + x;
            if (sx < 0 || sx >= GAME_W) continue;
            for (int y = 0; y < TILE; y++) {
                int sy = dy + y;
                if (sy < 0 || sy >= GAME_H) continue;
                fb[sx * GAME_H + sy] = src[y * TILE + x];
            }
        }
    } else {
        int x0 = dx * view_num / view_den;
        int y0 = dy * view_num / view_den;
        int dw = TILE * view_num / view_den;
        int dh = TILE * view_num / view_den;
        for (int ox = 0; ox < dw; ox++) {
            int sx = x0 + ox;
            if (sx < 0 || sx >= GAME_W) continue;
            int tx = ox * TILE / dw;
            for (int oy = 0; oy < dh; oy++) {
                int sy = y0 + oy;
                if (sy < 0 || sy >= GAME_H) continue;
                int ty = oy * TILE / dh;
                fb[sx * GAME_H + sy] = src[ty * TILE + tx];
            }
        }
    }
    if (t == T_WATER || t == T_FOUNTAIN) {
        int phase = (int)((water_anim / 4 + (dx / TILE) * 3 + dy / TILE) & 7);
        int wx = 2 + phase * 2;
        put_px(fb, dx + wx, dy + 5, C_WATER_L);
        put_px(fb, dx + wx + 1, dy + 6, RGB(220, 244, 240));
        put_px(fb, dx + wx + 2, dy + 5, C_WATER_L);
    }
}

static void draw_world_label(uint16_t *fb, int camx, int camy, int tx, int ty, const char *t)
{
    int len = 0;
    while (t[len]) len++;
    int x = ((tx * TILE + 8 - camx) * view_num / view_den) - len * 6;
    int y = ((ty * TILE - camy) * view_num / view_den) - 22;
    if (y < 42) y = 42;
    if (x < -len * 12 || x > GAME_W) return;
    int sn = view_num, sd = view_den;
    view_num = view_den = 1;
    draw_text_big_shadow(fb, x, y, t, C_WHITE);
    view_num = sn; view_den = sd;
}

static void draw_fishing(uint16_t *fb, int camx, int camy)
{
    int px = (player_x - camx + 8) * ZOOM_NUM / ZOOM_DEN;
    int py = (player_y - camy - 6) * ZOOM_NUM / ZOOM_DEN;
    view_num = view_den = 1;

    if (fish_state == FISH_WAIT) {
        draw_text_big_shadow(fb, px - 24, py - 32, "wait", C_WHITE);
        return;
    }
    if (fish_state == FISH_BITE) {
        draw_text_big_shadow(fb, px - 4, py - 34, "!", C_GOLD);
        draw_text_big_shadow(fb, px - 6, py - 16, "A", C_WHITE);
        return;
    }

    int x = 200, y = 50, w = 104, h = 172;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_HUD_L);
    fill_rect(fb, x, y + h - 1, w, 1, C_HUD_L);
    draw_text_big_shadow(fb, x + 8, y + 4, "fish", C_GOLD);

    int tx = x + 18, ty = y + 22;
    int tw = 18, th = 126;
    fill_rect(fb, tx, ty, tw, th, RGB(30, 42, 54));
    fill_rect(fb, tx, ty, tw, 1, C_HUD_L);

    int gy = ty + fish_green;
    fill_rect(fb, tx + 1, gy, tw - 2, fish_zone_h, RGB(90, 190, 100));

    int fy = ty + fish_pos;
    fill_rect(fb, tx - 4, fy, tw + 8, 7, RGB(230, 230, 240));
    fill_rect(fb, tx - 2, fy + 1, 6, 4, RGB(70, 90, 140));

    int cx = x + 58, cy = ty, cw = 10, ch = th;
    fill_rect(fb, cx, cy, cw, ch, RGB(40, 50, 40));
    int fill = ch * fish_catch / 100;
    fill_rect(fb, cx, cy + ch - fill, cw, fill, C_GOLD);

    draw_text_big_shadow(fb, x + 8, y + h - 38, "A hold", C_WHITE);
    draw_text_big_shadow(fb, x + 8, y + h - 20, "B quit", C_WHITE);
}

static void draw_world_labels(uint16_t *fb, int camx, int camy)
{
    static const struct { uint8_t scene, tx, ty; const char *text; } labels[] = {
        {SC_FARM, 5, 7, "home"},
        {SC_FARM, 9, 8, "shop"},
        {SC_FARM, 8, 10, "bin"},
        {SC_FARM, 38, 14, "town"},
        {SC_TOWN, 7, 9, "shop"},
        {SC_TOWN, 1, 14, "farm"},
        {SC_TOWN, 20, 28, "beach"},
        {SC_BEACH, 20, 2, "town"},
        {SC_BEACH, 6, 7, "rod"},
        {SC_BEACH, 28, 15, "sea"},
        {SC_HOME, 8, 3, "book"},
        {SC_SHOP, 18, 5, "pet"},
        {SC_TOWN, 20, 1, "mine"},
    };

    for (unsigned k = 0; k < sizeof(labels) / sizeof(labels[0]); k++) {
        if (labels[k].scene != scene_id) continue;
        draw_world_label(fb, camx, camy, labels[k].tx, labels[k].ty, labels[k].text);
    }

    if (scene_id == SC_TOWN) {
        static const uint8_t hx[3] = {15, 25, 8};
        static const uint8_t hy[3] = {9, 9, 22};
        for (int i = 0; i < 3; i++) {
            const char *w = npc_where(i);
            if (w[0] == 'h') {
                draw_world_label(fb, camx, camy, hx[i], hy[i], npc_def[i].name);
            }
        }
    }
}

static void pet_update(void)
{
    if (scene_id != SC_HOME) return;
    for (int i = 0; i < 2; i++) {
        if (!(pet_owned & (1 << i))) continue;
        if (i == 0 && cat_on_bed()) continue;
        if (pet_wait[i] > 0) { pet_wait[i]--; continue; }
        if ((pet_frame[i] & 3) == 0) {
            int nx = pet_x[i], ny = pet_y[i];
            int d = rnd() & 3;
            pet_dir[i] = (uint8_t)d;
            if (d == 0) ny += 2;
            else if (d == 1) ny -= 2;
            else if (d == 2) nx -= 2;
            else nx += 2;
            if (nx >= 32 && nx <= 270 && ny >= 52 && ny <= 200 && can_stand(nx, ny)) {
                pet_x[i] = nx;
                pet_y[i] = ny;
            } else {
                pet_wait[i] = 20;
            }
        }
        pet_frame[i]++;
        if ((pet_frame[i] % 120) == 0) pet_wait[i] = 40 + rnd() % 100;
    }

    if (!dino_unlocked) return;
    if (dino_wait > 0) { dino_wait--; return; }
    if ((dino_frame & 3) == 0) {
        int nx = dino_x, ny = dino_y;
        int d = rnd() & 3;
        if (d == 0) ny += 2;
        else if (d == 1) ny -= 2;
        else if (d == 2) nx -= 2;
        else nx += 2;
        if (nx >= 32 && nx <= 270 && ny >= 52 && ny <= 200 && can_stand(nx, ny)) {
            dino_x = nx;
            dino_y = ny;
        } else {
            dino_wait = 30;
        }
    }
    dino_frame++;
    if ((dino_frame % 150) == 0) dino_wait = 60 + rnd() % 100;
}

static void draw_dino(uint16_t *fb, int x, int y)
{
    fill_rect(fb, x + 2, y + 14, 12, 2, RGB(30, 50, 30));
    fill_rect(fb, x + 4, y + 6, 9, 8, RGB(86, 158, 82));
    fill_rect(fb, x + 5, y + 4, 7, 4, RGB(104, 178, 92));
    fill_rect(fb, x + 11, y + 8, 3, 3, RGB(86, 158, 82));
    fill_rect(fb, x + 12, y + 10, 3, 2, RGB(70, 136, 68));
    fill_rect(fb, x + 3, y + 7, 2, 2, RGB(70, 136, 68));
    fill_rect(fb, x + 7, y + 2, 2, 3, RGB(70, 136, 68));
    fill_rect(fb, x + 10, y + 1, 2, 2, RGB(70, 136, 68));
    put_px(fb, x + 10, y + 6, RGB(20, 30, 20));
    put_px(fb, x + 6, y + 9, RGB(180, 220, 140));
    fill_rect(fb, x + 5, y + 14, 2, 2, RGB(70, 136, 68));
    fill_rect(fb, x + 10, y + 14, 2, 2, RGB(70, 136, 68));
}

static void draw_pet_type(uint16_t *fb, int x, int y, int type)
{
    uint16_t out = RGB(44, 32, 24);
    fill_rect(fb, x + 3, y + 15, 10, 1, RGB(40, 30, 24));
    if (type == 1) {
        fill_rect(fb, x + 11, y + 8, 4, 6, out);
        fill_rect(fb, x + 12, y + 6, 3, 3, RGB(214, 146, 74));
        fill_rect(fb, x + 3, y + 7, 10, 7, out);
        fill_rect(fb, x + 4, y + 8, 9, 6, RGB(214, 146, 74));
        fill_rect(fb, x + 5, y + 9, 7, 4, RGB(234, 170, 96));
        fill_rect(fb, x + 4, y + 3, 8, 6, out);
        fill_rect(fb, x + 5, y + 4, 7, 5, RGB(224, 156, 84));
        fill_rect(fb, x + 4, y + 1, 2, 3, RGB(224, 156, 84));
        fill_rect(fb, x + 10, y + 1, 2, 3, RGB(224, 156, 84));
        fill_rect(fb, x + 5, y + 2, 1, 2, RGB(245, 190, 130));
        fill_rect(fb, x + 11, y + 2, 1, 2, RGB(245, 190, 130));
        fill_rect(fb, x + 7, y + 5, 2, 2, RGB(30, 26, 22));
        fill_rect(fb, x + 10, y + 5, 2, 2, RGB(30, 26, 22));
        put_px(fb, x + 9, y + 7, RGB(120, 54, 48));
        put_px(fb, x + 4, y + 8, RGB(246, 214, 170));
        put_px(fb, x + 11, y + 8, RGB(246, 214, 170));
        put_px(fb, x + 4, y + 11, RGB(246, 214, 170));
        put_px(fb, x + 11, y + 11, RGB(246, 214, 170));
        fill_rect(fb, x + 6, y + 9, 1, 2, RGB(180, 110, 54));
        put_px(fb, x + 2, y + 7, RGB(240, 240, 240));
        put_px(fb, x + 1, y + 8, RGB(240, 240, 240));
        put_px(fb, x + 13, y + 7, RGB(240, 240, 240));
        put_px(fb, x + 14, y + 8, RGB(240, 240, 240));
    } else {
        fill_rect(fb, x + 4, y + 5, 9, 8, out);
        fill_rect(fb, x + 5, y + 6, 8, 7, RGB(248, 244, 232));
        fill_rect(fb, x + 7, y + 3, 6, 5, out);
        fill_rect(fb, x + 8, y + 4, 5, 4, RGB(250, 246, 236));
        fill_rect(fb, x + 8, y + 1, 2, 3, RGB(214, 58, 52));
        fill_rect(fb, x + 10, y + 2, 1, 2, RGB(214, 58, 52));
        fill_rect(fb, x + 12, y + 5, 2, 2, RGB(238, 178, 60));
        fill_rect(fb, x + 12, y + 7, 1, 2, RGB(190, 48, 44));
        put_px(fb, x + 9, y + 4, RGB(30, 26, 22));
        fill_rect(fb, x + 5, y + 8, 4, 3, RGB(224, 218, 204));
        fill_rect(fb, x + 6, y + 9, 2, 2, RGB(200, 192, 176));
        fill_rect(fb, x + 5, y + 14, 2, 1, RGB(236, 176, 60));
        fill_rect(fb, x + 9, y + 14, 2, 1, RGB(236, 176, 60));
        put_px(fb, x + 4, y + 6, RGB(255, 255, 255));
    }
}
static void draw_pet_shop(uint16_t *fb)
{
    int x = 60, y = 48, w = 200, h = 144;
    fill_rect(fb, x, y, w, h, C_HUD);
    fill_rect(fb, x, y, w, 1, C_GOLD);
    fill_rect(fb, x, y + h - 1, w, 1, C_GOLD);
    draw_text_big_shadow(fb, x + 16, y + 8, "pet shop", C_GOLD);
    draw_gold(fb, x + 130, y + 8, coins);

    for (int i = 0; i < 2; i++) {
        int yy = y + 38 + i * 28;
        if (i == pet_shop_sel) fill_rect(fb, x + 8, yy - 4, w - 16, 24, C_HUD_L);
        draw_pet_type(fb, x + 16, yy - 4, i + 1);
        draw_text_big_shadow(fb, x + 42, yy, pet_name[i + 1], C_WHITE);
        if (pet_owned & (1 << i)) draw_text_big_shadow(fb, x + 140, yy, "have", C_GOLD);
        else draw_gold(fb, x + 140, yy, pet_price[i + 1]);
    }
    if (pet_shop_sel == 2) fill_rect(fb, x + 8, y + 94, w - 16, 20, C_HUD_L);
    draw_text_big_shadow(fb, x + 16, y + 98, "exit", C_WHITE);
    draw_text_big_shadow(fb, x + 16, y + h - 22, "A buy  B close", C_WHITE);
}

static int slime_tick;

static void mine_update(void)
{
    if (scene_id != SC_MINE) return;
    slime_tick++;
    if (slime_hit_cd > 0) slime_hit_cd--;
    for (int i = 0; i < 3; i++) {
        if (!slime_alive[i]) continue;
        if ((slime_tick & 3) == 0) {
            int dx = 0, dy = 0;
            if (slime_x[i] < player_x) dx = 1;
            else if (slime_x[i] > player_x) dx = -1;
            if (slime_y[i] < player_y) dy = 1;
            else if (slime_y[i] > player_y) dy = -1;
            if (dx && can_stand(slime_x[i] + dx, slime_y[i])) slime_x[i] += dx;
            if (dy && can_stand(slime_x[i], slime_y[i] + dy)) slime_y[i] += dy;
        }
        if (slime_hit_cd == 0 &&
            iabs(slime_x[i] - player_x) < 12 && iabs(slime_y[i] - player_y) < 12) {
            player_hp--;
            slime_hit_cd = 50;
            sound_play(SFX_FAIL);
            if (player_hp <= 0) {
                player_hp = 3;
                scene_set(SC_HOME);
                player_x = 10 * TILE;
                player_y = 10 * TILE;
                set_message("woke at home");
                return;
            }
            set_message("ouch");
        }
    }
}

static void draw_slime(uint16_t *fb, int sx, int sy)
{
    fill_rect(fb, sx + 3, sy + 14, 10, 2, RGB(30, 50, 30));
    fill_rect(fb, sx + 3, sy + 6, 10, 8, RGB(70, 140, 70));
    fill_rect(fb, sx + 4, sy + 4, 8, 3, RGB(90, 168, 82));
    fill_rect(fb, sx + 6, sy + 8, 2, 2, RGB(20, 30, 20));
    fill_rect(fb, sx + 10, sy + 8, 2, 2, RGB(20, 30, 20));
    fill_rect(fb, sx + 6, sy + 11, 4, 1, RGB(40, 70, 40));
    put_px(fb, sx + 5, sy + 6, RGB(140, 210, 120));
}

static void draw_dolphin(uint16_t *fb, int x, int y, int t)
{
    int jump = (t > 60) ? (120 - t) / 3 : t / 3;
    if (jump > 12) jump = 12;
    y -= jump;
    fill_rect(fb, x + 4, y + 8, 10, 4, RGB(80, 140, 196));
    fill_rect(fb, x + 8, y + 5, 7, 5, RGB(96, 160, 214));
    fill_rect(fb, x + 13, y + 7, 3, 2, RGB(70, 126, 180));
    fill_rect(fb, x + 2, y + 9, 3, 2, RGB(70, 126, 180));
    put_px(fb, x + 10, y + 7, C_WHITE);
    put_px(fb, x + 11, y + 7, RGB(20, 30, 40));
    put_px(fb, x + 5, y + 12, RGB(220, 240, 244));
    put_px(fb, x + 15, y + 6, RGB(220, 240, 244));
}

static void draw_dusk(uint16_t *fb)
{
    if (scene_indoor[scene_id]) return;
    int warm = 0, dark = 0;
    if (minutes >= 18 * 60 && minutes < 19 * 60 + 30) {
        warm = 30;
    } else if (minutes >= 19 * 60 + 30 && minutes < 21 * 60) {
        warm = 12;
        dark = 35;
    } else if (minutes >= 21 * 60 || minutes < 5 * 60) {
        dark = 50;
    } else {
        return;
    }

    uint8_t rmap[32], gmap[64], bmap[32];
    for (int i = 0; i < 32; i++) {
        int v = i;
        if (dark) v = v * (64 - dark) / 64;
        if (warm) v = v + (31 - v) * warm / 100;
        if (v > 31) v = 31;
        rmap[i] = (uint8_t)v;
        bmap[i] = (uint8_t)v;
    }
    for (int i = 0; i < 64; i++) {
        int v = i;
        if (dark) v = v * (64 - dark) / 64;
        if (warm) v = v + (32 - v) * warm / 100;
        if (v > 63) v = 63;
        gmap[i] = (uint8_t)v;
    }

    for (int x = 0; x < GAME_W; x++) {
        for (int y = 0; y < GAME_H; y++) {
            uint16_t c = fb[x * GAME_H + y];
            int r = (c >> 11) & 31;
            int g = (c >> 5) & 63;
            int b = c & 31;
            fb[x * GAME_H + y] = (uint16_t)((rmap[r] << 11) | (gmap[g] << 5) | bmap[b]);
        }
    }
}

void game_render(uint16_t *fb)
{
    if (state == ST_TITLE) {
        view_num = view_den = 1;
        draw_title(fb, title_t);
        return;
    }

    if (state == ST_WIN) {
        draw_ending(fb);
        return;
    }

    if (sleep_timer > 0) {
        fill_rect(fb, 0, 0, GAME_W, GAME_H, C_BLACK);
        int len = 0;
        while (sleep_text[len]) len++;
        draw_text_big_shadow(fb, (GAME_W - len * 12) / 2, 108, sleep_text, C_WHITE);
        return;
    }

    if (summary_open) {
        fill_rect(fb, 0, 0, GAME_W, GAME_H, C_BLACK);
        draw_summary(fb);
        return;
    }

    if (bin_open) {
        fill_rect(fb, 0, 0, GAME_W, GAME_H, C_BLACK);
        draw_bin(fb);
        return;
    }

    if (fade_timer > 10) {
        fill_rect(fb, 0, 0, GAME_W, GAME_H, C_BLACK);
        return;
    }

    if (scene_indoor[scene_id]) {
        view_num = view_den = 1;
    } else {
        view_num = ZOOM_NUM;
        view_den = ZOOM_DEN;
    }
    int vw = GAME_W * view_den / view_num;
    int vh = GAME_H * view_den / view_num;

    int camx = clamp(player_x + 8 - vw / 2, 0, scene_w[scene_id] * TILE - vw);
    int camy = clamp(player_y + 8 - vh / 2, 0, scene_h[scene_id] * TILE - vh);

    int tx0 = camx / TILE;
    int ty0 = camy / TILE;
    int tx1 = (camx + vw - 1) / TILE;
    int ty1 = (camy + vh - 1) / TILE;

    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            uint8_t t = map_get(tx, ty);
            if (scene_id == SC_FARM && (t == T_SOIL || t == T_SOIL_W)) {
                int idx = plot_index(tx, ty);
                if (idx >= 0 && plot_water[idx]) t = T_SOIL_W;
                else t = T_SOIL;
            }
            draw_tile(fb, tx * TILE - camx, ty * TILE - camy, t);
        }
    }

    if (scene_indoor[scene_id] && map_get(3, 3) == T_BED) {
        draw_bed(fb, 3 * TILE - camx, 3 * TILE - camy);
    }

    draw_world_labels(fb, camx, camy);

    if (scene_id == SC_BEACH) {
        for (int i = 0; i < 4; i++) {
            if (!(shell_mask & (1 << i))) {
                draw_shell(fb, shell_x[i] * TILE - camx, shell_y[i] * TILE - camy);
            }
        }
        if (dolphin_timer > 0) draw_dolphin(fb, 30 * TILE - camx, 12 * TILE - camy, dolphin_timer);
    }

    if (scene_id == SC_FARM) {
        for (int ty = ty0; ty <= ty1; ty++) {
            for (int tx = tx0; tx <= tx1; tx++) {
                int idx = plot_index(tx, ty);
                if (idx >= 0 && plot_crop[idx]) {
                    int cid = plot_crop[idx] - 1;
                    int cx = tx * TILE - camx;
                    int cy = ty * TILE - camy;
                    draw_crop(fb, cx, cy, cid, plot_growth[idx], plot_water[idx]);
                    if (plot_growth[idx] >= crops[cid].days) {
                        put_px(fb, cx + 7, cy - 3, C_GOLD);
                        put_px(fb, cx + 6, cy - 2, C_GOLD);
                        put_px(fb, cx + 8, cy - 2, C_GOLD);
                        put_px(fb, cx + 7, cy - 1, C_GOLD);
                        put_px(fb, cx + 7, cy - 4, RGB(255, 255, 220));
                    }
                }
            }
        }
    }

    if (scene_id == SC_MINE) {
        for (int i = 0; i < 3; i++) {
            if (slime_alive[i]) draw_slime(fb, slime_x[i] - camx, slime_y[i] - camy);
        }
    }

    for (int i = 0; i < 3; i++) {
        if (npcs[i].scene == scene_id) {
            int nsx = npcs[i].x - camx;
            int nsy = npcs[i].y - camy;
            draw_npc(fb, nsx, nsy, i);
            char mk = story_marker(i);
            if (mk) {
                char m[2] = { mk, 0 };
                draw_text_big_shadow(fb, nsx + 3, nsy - 34, m, C_GOLD);
            }
        }
    }

    if (scene_id == SC_HOME) {
        for (int i = 0; i < 2; i++) {
            if (!(pet_owned & (1 << i))) continue;
            if (i == 0 && cat_on_bed()) continue;
            draw_pet_type(fb, pet_x[i] - camx, pet_y[i] - camy, i + 1);
        }
        if (cat_on_bed()) draw_pet_type(fb, 4 * TILE - camx, 3 * TILE - camy, 1);
        if (dino_unlocked) draw_dino(fb, dino_x - camx, dino_y - camy);
    }

    {
        int psx = player_x - camx;
        int psy = player_y - camy;
        int ptx = (player_x + 8) / TILE;
        int pty = (player_y + 12) / TILE;
        uint8_t under = map_get(ptx, pty);
        uint16_t sc = (under == T_SOIL || under == T_SOIL_W) ? RGB(66, 46, 30)
                                                             : (scene_indoor[scene_id] ? C_FLOOR_D : C_GRASS_D);
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 3; j++)
                put_px(fb, psx + 3 + i, psy + 14 + j, sc);
    }

    draw_player(fb, player_x - camx, player_y - camy);
    view_num = view_den = 1;
    if (!scene_indoor[scene_id]) draw_weather(fb);
    draw_dusk(fb);

    draw_hud(fb);
    if (fish_state != FISH_IDLE) {
        draw_fishing(fb, camx, camy);
        return;
    }
    if (pet_shop_open) {
        draw_pet_shop(fb);
        return;
    }
    if (menu_open) {
        draw_menu(fb);
        return;
    }
    if (status_open) draw_status(fb);
    else if (shop_open) draw_shop(fb);
    else if (dlg_open) draw_dialogue(fb);
    else {
        draw_message(fb);
        draw_prompt(fb);
    }

    if (fade_timer > 0) fill_rect(fb, 0, 0, GAME_W, GAME_H, C_BLACK);
}

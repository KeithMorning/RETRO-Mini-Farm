#ifndef SAVE_H
#define SAVE_H

#include <stdint.h>

#define SAVE_MAGIC   0x4d464d31u
#define SAVE_VERSION 6u

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t crc;

    int16_t year;
    int16_t season;
    int16_t day;
    int16_t minutes;
    int16_t coins;
    int16_t energy;
    int16_t bin_value;

    int16_t scene;
    int16_t px;
    int16_t py;
    int16_t dir;

    uint8_t selected;
    uint8_t weather;
    uint8_t flags;
    uint8_t pad0;
    uint8_t water_can;

    uint8_t seeds[16];
    uint8_t produce[16];

    uint8_t crop_id[24];
    uint8_t crop_growth[24];
    uint8_t crop_water[24];

    uint8_t npc_points[3];
    uint8_t npc_talked[3];
    uint8_t npc_gifted[3];

    uint8_t pad[3];
    uint8_t bin_items[16];
    uint8_t story[3];
    uint8_t shell_count;
    uint8_t ore_count;
    uint8_t player_hp;
    uint8_t mine_floor;
    uint8_t mine_pad;
    uint8_t pet_owned;
    uint8_t pet_aff[2];
    uint8_t pet_fed;
    uint8_t dino_unlocked;
    uint8_t pet_pad[3];
    uint8_t seeds_ext[2];
    uint8_t produce_ext[2];
    uint8_t bin_ext[2];
    uint8_t tilled[3];
    uint8_t pad2[3];
} SaveData;

int save_read(SaveData *d);
int save_write(const SaveData *d);

#endif

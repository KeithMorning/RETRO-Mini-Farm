#include "save.h"

#include <string.h>
#include <libopencm3/stm32/flash.h>

#define SAVE_ADDR 0x08060000u
#define SAVE_SECTOR 7

_Static_assert(sizeof(SaveData) % 4 == 0, "SaveData must be word aligned");

static uint32_t crc32(const uint8_t *p, int n)
{
    uint32_t c = 0xFFFFFFFFu;
    for (int i = 0; i < n; i++) {
        c ^= p[i];
        for (int k = 0; k < 8; k++) {
            c = (c >> 1) ^ (0xEDB88320u & (~((c & 1) - 1)));
        }
    }
    return ~c;
}

static uint32_t save_crc(SaveData d)
{
    d.crc = 0;
    return crc32((const uint8_t *)&d, sizeof(d));
}

/* v1 was 156 bytes. v2 was 172 bytes with bin_items. */
#define SAVE_V1_SIZE 156
#define SAVE_V2_SIZE 172
#define SAVE_V3_SIZE 176
#define SAVE_V4_SIZE 180
#define SAVE_V5_SIZE 188

static void migrate_legacy(SaveData *d)
{
    /* v5 and older had 16 item slots; items 14/15 were fossil/egg.
       The 18-slot layout uses 14/15 for winter crops and 16/17 for those
       special items. Old farms also had all 24 field tiles pre-tilled. */
    d->seeds_ext[0] = 0;
    d->seeds_ext[1] = 0;
    d->produce_ext[0] = d->produce[14];
    d->produce_ext[1] = d->produce[15];
    d->produce[14] = 0;
    d->produce[15] = 0;
    d->bin_ext[0] = d->bin_items[14];
    d->bin_ext[1] = d->bin_items[15];
    d->bin_items[14] = 0;
    d->bin_items[15] = 0;
    memset(d->tilled, 0xFF, sizeof(d->tilled));
    d->pad2[0] = d->pad2[1] = d->pad2[2] = 0;
    if (((d->selected >> 5) & 7) == 6) {
        int id = d->selected & 31;
        if (id == 14 || id == 15) d->selected = (uint8_t)((6 << 5) | (id + 2));
    }
}

int save_read(SaveData *d)
{
    const uint8_t *p = (const uint8_t *)SAVE_ADDR;
    uint32_t magic, version, crc;
    memcpy(&magic, p, 4);
    memcpy(&version, p + 4, 4);
    if (magic != SAVE_MAGIC) return 0;

    if (version == 1) {
        uint8_t old[SAVE_V1_SIZE];
        memcpy(old, p, sizeof(old));
        memcpy(&crc, old + 8, 4);
        memset(old + 8, 0, 4);
        if (crc32(old, sizeof(old)) != crc) return 0;
        memset(d, 0, sizeof(*d));
        memcpy(d, old, sizeof(old));
        migrate_legacy(d);
        return 1;
    }

    if (version == 2) {
        uint8_t old[SAVE_V2_SIZE];
        memcpy(old, p, sizeof(old));
        memcpy(&crc, old + 8, 4);
        memset(old + 8, 0, 4);
        if (crc32(old, sizeof(old)) != crc) return 0;
        memset(d, 0, sizeof(*d));
        memcpy(d, old, sizeof(old));
        migrate_legacy(d);
        return 1;
    }

    if (version == 5) {
        uint8_t old[SAVE_V5_SIZE];
        memcpy(old, p, sizeof(old));
        memcpy(&crc, old + 8, 4);
        memset(old + 8, 0, 4);
        if (crc32(old, sizeof(old)) != crc) return 0;
        memset(d, 0, sizeof(*d));
        memcpy(d, old, sizeof(old));
        migrate_legacy(d);
        return 1;
    }

    if (version == 4) {
        uint8_t old[SAVE_V4_SIZE];
        memcpy(old, p, sizeof(old));
        memcpy(&crc, old + 8, 4);
        memset(old + 8, 0, 4);
        if (crc32(old, sizeof(old)) != crc) return 0;
        memset(d, 0, sizeof(*d));
        memcpy(d, old, sizeof(old));
        migrate_legacy(d);
        return 1;
    }

    if (version == 3) {
        uint8_t old[SAVE_V3_SIZE];
        memcpy(old, p, sizeof(old));
        memcpy(&crc, old + 8, 4);
        memset(old + 8, 0, 4);
        if (crc32(old, sizeof(old)) != crc) return 0;
        memset(d, 0, sizeof(*d));
        memcpy(d, old, sizeof(old));
        migrate_legacy(d);
        return 1;
    }

    if (version != SAVE_VERSION) return 0;
    memcpy(d, p, sizeof(*d));
    if (save_crc(*d) != d->crc) return 0;
    return 1;
}

int save_write(const SaveData *src)
{
    SaveData d = *src;
    d.magic = SAVE_MAGIC;
    d.version = SAVE_VERSION;
    d.crc = save_crc(d);

    flash_unlock();
    flash_erase_sector(SAVE_SECTOR, FLASH_CR_PROGRAM_X32);
    flash_program(SAVE_ADDR, (const uint8_t *)&d, sizeof(d));
    flash_lock();
    return 1;
}

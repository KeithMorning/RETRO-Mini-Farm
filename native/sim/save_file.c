#include "save.h"
#include <stdio.h>

int save_read(SaveData *d)
{
    FILE *f = fopen("save.dat", "rb");
    if (!f) return 0;
    size_t n = fread(d, 1, sizeof(*d), f);
    fclose(f);
    if (n != sizeof(*d)) return 0;
    if (d->magic != SAVE_MAGIC || d->version != SAVE_VERSION) return 0;
    return 1;
}

int save_write(const SaveData *d)
{
    SaveData x = *d;
    x.magic = SAVE_MAGIC;
    x.version = SAVE_VERSION;
    FILE *f = fopen("save.dat", "wb");
    if (!f) return 0;
    fwrite(&x, 1, sizeof(x), f);
    fclose(f);
    return 1;
}

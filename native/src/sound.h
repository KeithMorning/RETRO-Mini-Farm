#ifndef SOUND_H
#define SOUND_H

enum {
    SFX_NONE = 0,
    SFX_CLICK,
    SFX_PLANT,
    SFX_WATER,
    SFX_HARVEST,
    SFX_COIN,
    SFX_GIFT,
    SFX_DOOR,
    SFX_SLEEP,
    SFX_UPGRADE,
    SFX_CATCH,
    SFX_FAIL,
    SFX_ERROR,
    SFX_PET
};

void sound_init(void);
void sound_play(int id);

#endif

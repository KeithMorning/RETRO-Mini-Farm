# RETRO Mini Farm v1.0.0

A complete, child-friendly farming game for the
**ELECFREAKS RETRO Arcade for Education**.

Grow crops through four seasons, make three friends, fish at the beach,
upgrade a house, adopt a cat and a chicken, and unlock a little mine.

## Download

- `retro-mini-farm-v1.0.0.uf2` — ready to flash
- `retro-mini-farm-v1.0.0.zip` — UF2 + quick start + README + license + checksums

## Install

1. Hold **Menu + B** until the handheld reboots and `ARCADE-F4` appears.
2. Copy `retro-mini-farm-v1.0.0.uf2` to the `ARCADE-F4` drive.
3. The device reboots automatically.

The save is in a separate flash sector. Normal firmware upgrades keep the
farm.

## Goal

There is no 500g finish line. Bring the town back to life: finish all three
storylines and reach **3 hearts with Mira, Tom and Lily** before the end of
year one. A best-friends ending is available for 5 hearts and the dinosaur.

## Highlights

- **Four seasons × 28 days**, 16 crops (4 per season), rain and snow
- **Three NPCs** — Mira, Tom and Lily — with schedules, gifts, five hearts
  and three two-chapter storylines
- **Beach and fishing**: shells, rod, four fish and a green-bar minigame
- **Five house levels** and water-can upgrades
- **Cat, chicken and dinosaur**: daily feeding, half hearts, daily gifts,
  and a dinosaur that digs ore
- **Mine**: unlocked after all three stories, with sword, copper ore and slimes
- **30-slot bag** with tools, remembered selection and seed/produce names
- **Hoe tilling**: turn the fenced grass into soil, and the tilled layout is saved
- **Eight hidden moments**: fountain wish, dolphin, cat on bed, golden fish,
  fossil, NPC secrets, campfire and a mailbox letter
- New **procedural startup animation** and a dedicated title illustration
- PWM sound effects, save v6, automatic migration from v1–v5

## Controls

| Button | Action |
|---|---|
| D-pad | Move / cursor |
| A | Use selected tool/item, talk, select in bag |
| B | Gift selected crop/fish near NPC, close |
| Menu | Pause menu; bag is Menu → bag |
| Menu + B | Enter UF2 bootloader |

## Save compatibility

The current save format is **v6**. Saves from v1–v5 are migrated
automatically. A save is written when you sleep, use Menu → save, upgrade,
or complete a story quest.

## Notes

- This repository is source-only; prebuilt UF2 files are attached to this
  GitHub Release.
- The ARM toolchain is not included. Source build instructions are in the
  repository README.
- The mine currently has one floor.

## Credits

- Hardware: ELECFREAKS
- Low-level STM32 library: libopencm3
- Toolchain: xPack ARM GNU Toolchain
- Game code and procedural art: Keith Xi

Original code and art are MIT licensed. libopencm3 remains under its own
LGPL/GPL licenses.

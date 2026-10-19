# AI Agent Handoff — RETRO Mini Farm

This document is written for another coding agent that has shell and file
access to this repository but **no prior context**.

Goal: reproduce the build environment, build the firmware, run the simulator,
optionally flash the handheld, and safely modify the game.

The repository is **source-only**. It deliberately does not commit:

- the 1.3 GB ARM toolchain
- `native/build/`
- `native/sim/out/`
- simulator binaries
- `firmware/backup/`
- prebuilt UF2 files
- `native/third_party/` (fetched by `make deps`)

A user who receives the repo must install the toolchain (or point `TOOLCHAIN`
at it), then build.

---

## 1. Operating rules

1. **Do not flash the handheld unless the user explicitly asks.**
   Flashing writes to a USB mass-storage bootloader volume.
2. **Never erase or move save sector 7 (`0x08060000`).**
   App region is `0x08010000..0x0807FFFF`; save is outside it.
3. Do not rewrite `native/src/game.c` from scratch. It is large and shared by
   firmware and host simulator. Use targeted edits.
4. Build with `-Wall -Wextra`; leave **zero warnings**.
5. Keep in-game text lowercase, short and 2×. `A` and `B` button labels may be
   uppercase.
6. Keep the control contract:
   - **A** = use selected item/tool, talk, or select in bag
   - **B** = gift selected crop/fish near NPC, or close panel
   - **Menu** = pause menu
   - bag is only opened from **Menu → bag**
7. Preserve save compatibility. Current format is **v6**.
8. Keep static memory in mind:
   - RGB565 framebuffer: 320 × 240 × 2 = 150 KB
   - each 16×16 tile: 512 B of RAM
9. No sprinklers (explicit design decision).

---

## 2. Target hardware

| Item | Value |
|---|---|
| Device | ELECFREAKS RETRO Arcade for Education |
| MCU | STM32F412RE, Cortex-M4F |
| Clock | 96 MHz from HSI PLL; 16 MHz fallback |
| RAM | 256 KB |
| Flash | 512 KB total; 448 KB app region; 64 KB bootloader |
| Display | ILI9341, 320×240, 8-bit parallel / FSMC |
| Input | A/B/Menu + 4 directions |
| Save | internal flash sector 7 at `0x08060000` |
| USB | UF2 mass-storage bootloader, volume `ARCADE-F4` |

### Pin map (from `native/src/main.c`)

| Function | Pin |
|---|---|
| LCD CS | PB12 |
| LCD RST | PB7 |
| LCD backlight | PB9 |
| FSMC data/control | PA2–PA5, PB14, PC2/3/5/6/11/12 |
| Button A | PC0 |
| Button B | PC1 |
| Button left | PA15 |
| Button right | PC9 |
| Button up | PB5 |
| Button down | PB2 |
| Button menu | PC13 |
| Audio PWM | PA8 / TIM1_CH1 |
| Audio amp enable | PC10 |

LCD command/data window is at `0x60000000` / `0x60000001`.

The panel is rotated; the framebuffer is stored in **panel order**:

```c
fb[x * GAME_H + y]
```

not the usual row-major order.

### UF2

| Item | Value |
|---|---|
| UF2 base | `0x08010000` |
| UF2 family | `0x57755A57` |
| App region | 448 KB |
| Payload block | 256 bytes, UF2 block 512 bytes |

`native/tools/mkuf2.py` generates the UF2; `tools/uf2-info.py` can inspect one.

---

## 3. Quick start

```bash
cd native
make -j4
```

Output:

```text
native/build/app.uf2
```

Host simulator:

```bash
cd native
make sim
```

Screenshots are written to `native/sim/out/` and are not committed.

Regression tests:

```bash
cd native
make test
```

Flash only when asked:

```bash
cd native
make flash
# or copy build/app.uf2 to /Volumes/ARCADE-F4/
```

---

## 4. Toolchain / environment setup

### Required

- macOS or Linux
- `make`
- Python 3
- `cc` for the host simulator (clang or gcc)
- ARM GNU toolchain (xPack is what this project was developed with)

The Makefile defaults to:

```text
toolchain/xpack-arm-none-eabi-gcc-15.2.1-1.1/bin
```

If the toolchain is elsewhere:

```bash
make TOOLCHAIN=/path/to/arm-none-eabi/bin
```

`native/third_party/` is **not committed**. The Makefile clones libopencm3
`v0.8.0` into it and builds `lib/libopencm3_stm32f4.a` on first use:

```bash
cd native
make deps
make -j4
```

Do not commit `native/third_party/`; it is ignored intentionally.

Compile flags require hard-float:

```text
-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
```

Do not remove these; the floating-point ABI must match libopencm3.

---

## 5. Build and flash

### Build

```bash
cd native
make clean
make -j4
```

Expected:

- no warnings
- `build/app.elf`
- `build/app.bin`
- `build/app.uf2`

Approximate current size: **~74 KB app**, **~188 KB RAM used**.
There is a lot of flash/headroom; RAM is the tighter budget.

### Bootloader

1. Hold **Menu + B** until the device reboots.
2. USB drive `ARCADE-F4` appears for a short window.
3. Copy `native/build/app.uf2` to the root.
4. Device reboots automatically.

macOS:

```bash
cp native/build/app.uf2 /Volumes/ARCADE-F4/
```

or:

```bash
cd native
make flash
```

Linux mount points vary (`/media/$USER/ARCADE-F4`, `/run/media/...`).

### Verify a flash

```bash
shasum -a 256 native/build/app.uf2
```

Optionally read back `/Volumes/ARCADE-F4/app.uf2` and compare hashes.

### Save safety

- Save is sector 7, outside the app region.
- Do not add a linker section or erase command targeting sector 7.
- The bootloader UF2 update normally leaves it alone.
- A full chip erase or ST-Link mass erase would destroy the save.

---

## 6. Repository map

```text
AGENTS.md                  short agent rules
README.md                  English README
README.zh-CN.md            Chinese README
docs/
  AI-HANDOFF.md            this file
  build-flash.md           build/flash guide
  game-design.md           original design notes
  ...
  images/                  README screenshots
firmware/                  historical hardware tests (do not need)
native/
  Makefile
  flash.sh
  ld/app.ld
  src/
    main.c                 device layer
    game.c                 game core
    game.h                 public/debug API
    save.c                 flash + CRC + migrations
    save.h                 SaveData
    font8.c                6×8 font
    sound.h                SFX ids
  sim/
    main.c                 screenshot tour
    save_file.c            real-file save backend
    save_stub.c            no-op save backend
    sound_stub.c           no-op audio
    *_test.c               regression tests
    run_tests.sh           test runner
  tools/
    fb2png.py              raw RGB565 -> PNG
    mkuf2.py               bin -> UF2
  third_party/libopencm3   vendored library
tools/uf2-info.py          UF2 inspector
```

---

## 7. Architecture

### Layers

- `game.c`: pure-ish game logic and rendering into a `uint16_t` framebuffer.
  It knows nothing about STM32 registers.
- `main.c`: hardware only:
  - clock/PLL setup
  - FSMC LCD init and blit
  - buttons
  - PWM audio
  - main loop: `game_update()` → optional `game_render()` + `lcd_blit()`
- `save.c`: sector erase/program, CRC, version migration.
- `font8.c`: fixed 6×8 font.
- `sound.h`: SFX enum shared by game and device.

### Main loop contract

```c
Input in = read_buttons();
if (game_update(&in)) {
    game_render(fb);
    lcd_blit();
}
```

`game_update()` returns non-zero when a redraw is needed.
`game_render(fb)` fills the 320×240 framebuffer.

### Scenes

```c
SC_FARM, SC_TOWN, SC_HOME, SC_SHOP,
SC_MIRA, SC_TOM, SC_LILY, SC_BEACH, SC_MINE
```

Tiles are 16×16. Farm/town/beach are 40×30; interiors are 20×15.

Outdoor view uses a 1.25× world scale (`view_num/view_den`), interiors 1×.
`view_num == view_den` is the unscaled UI mode. Always reset scaling to 1
before HUD/menu/dialogue rendering.

### Rendering primitives

- `put_px(fb, x, y, color)` respects current world scale
- `fill_rect(...)`
- `draw_tile(...)`
- `draw_person(...)`
- `draw_crop(...)`
- `draw_fish_icon`, `draw_item_icon`, `draw_tool_icon`
- `draw_hud`, `draw_status`, `draw_bag`, `draw_shop`, `draw_dialogue`

Do not draw text with direct framebuffer writes; use the text helpers so
scaling and shadow work.

---

## 8. Game systems

### Farming

- 4 seasons × 28 days
- 16 crops (ids 0–15; ids 16 = fossil, 17 = egg as special items)
- `plot_crop/growth/water[24]`
- Plant only with a selected seed of the current season
- Water only with the watering can selected
- Harvest does **not** change the selected item
- Weather rain waters crops overnight

### Inventory / tools

- 30-slot bag, 6×5
- slots 0–3 fixed tools: hoe, can, rod, sword
- then current-season seeds, all produce, fish
- selected item is packed into `SaveData.selected`:
  `(kind << 5) | id`
- kinds: `IT_HOE`, `IT_CAN`, `IT_ROD`, `IT_SWORD`, `IT_SEED`,
  `IT_CROP`, `IT_FISH`
- `bag_find_selected()` maps the current selection to the bag cursor,
  including empty stacks, so the cursor never jumps to the hoe
- bag only opens from Menu; B is **not** a global bag key

### NPCs / story

- Mira, Tom, Lily
- 5 hearts; points 0–100; thresholds 40 and 80
- favorites and 2–4 page dialogue
- 3 two-chapter storylines; stages 0–4
- story markers `!` / `?`
- quests consume produce/fish/shells
- A talk; B gift selected crop/fish

### Pets

- cat and chicken can both be owned
- `pet_owned` bits; `pet_aff[2]` 0–100; 2 feeds = 1 heart (half hearts shown)
- cat bowl round (`T_BOWL`), chicken bowl square (`T_BOWL2`)
- full cat: daily fish; full chicken: daily egg
- both full: `dino_unlocked`; dino digs one ore per sleep
- save v6 fields: `pet_owned`, `pet_aff`, `pet_fed`, `dino_unlocked`,
  `seeds_ext[2]`, `produce_ext[2]`, `bin_ext[2]`, `tilled[3]`;
  `pet_pad[0]` stores rod/sword levels

### Mine

- unlocked when `story[0..2] == 4`
- town north door → `SC_MINE`
- **two floors**; ladder at `(10,2)` switches `mine_floor`
- sword selected; A breaks rocks/ore or hits slimes
- `ore_count`, `player_hp`; death returns home
- rod / sword upgrades add fishing zone and slime damage
- upgrade page shows rod only after purchase and sword after mine unlock
- all of this is in `game.c`; no separate module

### Easter eggs

Secrets are bits in `SaveData.mine_pad`:

| Bit | Meaning |
|---:|---|
| 1 | campfire |
| 2 | mailbox letter |
| 4 | golden fish found |
| 8 | golden fish NPC line told |
| 16 | fossil found |

Daily-only flags (`wish_done`, `dolphin_done`, `cat_bed_block`) are static,
not saved.

---

## 9. Save format

`native/src/save.h`:

- `SAVE_MAGIC 0x4d464d31`
- current `SAVE_VERSION 5`
- `SaveData` is 200 bytes, 4-byte aligned
- CRC32 over the whole struct with `crc = 0`
- stored at `0x08060000`, sector 7

Sizes/migrations:

| Version | Size | Notes |
|---:|---:|---|
| 1 | 156 | no bin items |
| 2 | 172 | bin items |
| 3 | 176 | story + shells |
| 4 | 180 | ore/hp/mine |
| 5 | 188 | pets/dino + secrets shared `mine_pad` |
| 6 | 200 | 18 item slots + tilled field mask |

`save_read()` accepts v1–v6 and zero-fills/migrates new fields.
When adding fields:

1. Append to the struct.
2. Keep the old struct size as `SAVE_Vx_SIZE`.
3. Add a migration branch that reads the old prefix and CRC-checks it.
4. Bump `SAVE_VERSION`.
5. Verify `sizeof(SaveData) % 4 == 0`.
6. Test an old save if possible.

Never write a save with a version older than `SAVE_VERSION`.

---

## 10. Common changes

### Add a tile

1. Add to the tile enum in `game.c`.
2. Add a `case` in `tiles_build()`.
3. Add it to `solid()` if needed.
4. Place it in a `build_*()` map.
5. Use `T_COUNT` for arrays; tile images cost 512 B RAM each.

### Add a crop

1. Add a `CropDef` to `crops[16]`.
2. Keep within an existing season block or adjust `season_crop_count`.
3. Ensure `draw_item_icon` handles its shape.
4. If it is not a real crop, do not put it in season counts.

### Add an NPC / schedule

- add to `npc_def`, schedules, `looks`, doors, labels
- `npc_where()`, story arrays and `hearts` page assume 3 NPCs; more work
  is needed to generalize those arrays.

### Add a scene

1. Add `SC_*` before `SC_COUNT`.
2. Add a tile buffer and scene width/height/indoor entry.
3. Build the map.
4. Add `Door` entries both ways.
5. Add a debug scene enum in `game.h`.
6. Add labels/prompts if needed.

### Add a save field

See section 9. Do not reuse `mine_pad`/`pet_pad` without documenting it.

### Add a sound

- add SFX enum in `sound.h`
- add case in `main.c:sound_play`
- call `sound_play(SFX_...)` from game logic
- simulator supplies `sim/sound_stub.c`

---

## 11. Testing

```bash
cd native
make test
make sim
```

`make test` compiles and runs:

- `save_test`
- `gameplay_test`
- `menu_test`
- `upgrade_test`
- `season_test`
- `beach_test`
- `pet_test`
- `story_test`
- `mine_test`
- `easter_test`
- `goal_test` (normal and best year-end endings)

`save_test` and `menu_test` use `sim/save_file.c`; the others use
`sim/save_stub.c`.

The simulator uses `sim/main.c` and `sim/save_stub.c`/`sim/sound_stub.c`;
`make sim` also builds all frames and a contact sheet.

### Manual acceptance

1. Boot shows the **title screen** first, even with an existing save.
2. Press A → play.
3. Move, plant, water, sleep.
4. Menu → bag: cursor is on the last selected item.
5. A selects; B closes.
6. Harvest does not change the selected item.
7. NPC `A talk` / `B gift`.
8. House/water upgrades.
9. Story journal via Menu → hearts → A next.
10. Mine after three stories.
11. Pets/dino.
12. Save, power cycle, load.

### Simulator screenshots to inspect

```text
sim/out/t0_title.png
sim/out/t1_farm.png
sim/out/t2_town.png
sim/out/t6_home.png
sim/out/m2_bag.png
sim/out/b6_game.png
sim/out/mn2_rock.png
sim/out/p2_pet_shop.png
sim/out/p8_dino.png
sim/out/m10_stories.png
sim/out/e2_campfire.png
```

---

## 12. Known pitfalls

- `fb[x * GAME_H + y]` is panel order, not row-major.
- `view_num/view_den` scaling must be reset to 1 for UI.
- `fill_rect`/`put_px` scale with world view; do not use them for UI while
  world scale is active.
- `tile_img` is RAM, not flash.
- `draw_tile` samples tile art; tile art is not a bitmap asset file.
- Font is 6×8; 2× text is 12×16. Check panel widths.
- `make sim` deletes `sim/out` before running.
- Save flash writes block; don't write every frame.
- Sound PWM uses TIM1; don't reuse TIM1 for something else.
- `start_fade()` refuses mine entry until all three stories are complete.
- `game_init()` always starts at `ST_TITLE`, even if a save loaded.
- `game_update()` returns dirty; don't force full redraw unless needed.
- The toolchain is intentionally not in the repo; don't try to commit it.
- The vendored libopencm3 must remain under its own LGPL/GPL terms.

---

## 13. Handoff acceptance

An agent is done when:

- [ ] `make clean && make -j4` succeeds with no warnings
- [ ] `make test` passes
- [ ] `make sim` produces screenshots
- [ ] no toolchain or build artifacts are staged for git
- [ ] if flashing was requested: UF2 readback SHA matches
- [ ] if the user had a save: sleep/save/load still works
- [ ] README and this document still describe reality

If something does not fit, prefer a small targeted change and a new regression
test over rewriting `game.c`.

# AGENTS.md — RETRO Mini Farm

This repository is a **source-only** bare-metal C game for the
**ELECFREAKS RETRO Arcade for Education** (STM32F412RE).

Before changing anything, read:

1. [`docs/AI-HANDOFF.md`](docs/AI-HANDOFF.md) — full environment, hardware,
   architecture, save format and operating rules.
2. [`README.md`](README.md) — player-facing features and controls.
3. [`docs/build-flash.md`](docs/build-flash.md) — build and UF2 flashing.

## Non-negotiables

- **Do not flash the device unless the user explicitly asks you to.**
  Flashing is a write to the handheld's bootloader volume.
- **Do not erase or relocate save sector 7** (`0x08060000`). It is outside the
  app region (`0x08010000..0x0807FFFF`).
- **Do not rewrite `native/src/game.c` from scratch.** It is the shared game
  core for firmware and simulator; make targeted edits.
- Keep UI text **lowercase**, short, and **2× font**. `A` / `B` may be uppercase.
- Keep the current control contract:
  - **A** = use selected item/tool, talk, or select in bag
  - **B** = gift near NPC / close panel
  - **Menu** = pause menu; **bag is only opened via Menu → bag**
- Preserve save compatibility. The current format is **v5**; add a migration
  path when changing `SaveData`.
- Build with `-Wall -Wextra` and leave **zero warnings**.
- Keep static memory use in mind: 320×240 RGB565 framebuffer is ~150 KB;
  each 16×16 tile image costs 512 B of RAM.
- No sprinklers (design decision).

## Fast commands

```bash
cd native
make deps         # clone pinned libopencm3 v0.8.0
make -j4          # firmware -> build/app.uf2
make sim          # host simulator + PNG frames in sim/out/
make test         # host regression tests
make clean
```

Toolchain default:

```text
toolchain/xpack-arm-none-eabi-gcc-15.2.1-1.1/bin
```

Override with `make TOOLCHAIN=/path/to/bin`.

## Where things live

| Path | Purpose |
|---|---|
| `native/src/game.c` | all game logic, rendering, scenes, UI, story, pets, mine |
| `native/src/main.c` | LCD/FSMC, buttons, clock, PWM audio, main loop |
| `native/src/save.c` | flash sector 7, CRC, v1–v5 migration |
| `native/src/save.h` | `SaveData` layout |
| `native/src/game.h` | public game/debug API |
| `native/src/font8.c` | 6×8 bitmap font |
| `native/sim/` | host simulator, stubs, regression tests |
| `native/third_party/` | fetched by `make deps`; not committed |
| `docs/images/` | README screenshots (assets only) |

## Before saying you are done

```bash
cd native
make clean && make -j4
make test
make sim
```

Then inspect at least:

- `sim/out/t0_title.png`
- `sim/out/t1_farm.png`
- `sim/out/m2_bag.png`
- `sim/out/p8_dino.png`
- `sim/out/mn2_rock.png`

Report the build SHA if you flashed anything.
